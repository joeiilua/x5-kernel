/*
 * Horizon Robotics
 *
 *  Copyright (C) 2020 Horizon Robotics Inc.
 *  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/kobject.h>

#include "socinfo.h"

#define SOCINFO_NAME		"socinfo"
#define BUF_LEN		128

struct _reg_info {
	void __iomem *map;
	/* addr, size, offset, mask */
	u32 reg_info[4];
};

struct _DDR_vendor_info {
	u8 vendor_id;
	char *vendor_name;
};

const char *soc_id;
const char *bootmode;
const char *socuid;
const char *origin_board_id;
const char *board_id;
const char *ddr_vendor;
const char *ddr_type;
const char *ddr_freq;
const char *ddr_size;
const char *ddr_part_num;
const char *som_name;
const char *base_board_name;
const char *board_name;
const char *chip_id;
static unsigned int secure_chip = 0;
static void __iomem *sec_flag_addr;
struct _reg_info g_bak_slot_info;
struct _reg_info g_boot_count;

const static struct _DDR_vendor_info ddr_vendor_info[] = {
	{
		.vendor_id = 0x1,
		.vendor_name = "Samsung",
	},
	{
		.vendor_id = 0x5,
		.vendor_name = "Nanya",
	},
	{
		.vendor_id = 0x6,
		.vendor_name = "SK hynix",
	},
	{
		.vendor_id = 0x8,
		.vendor_name = "Winbond",
	},
	{
		.vendor_id = 0x9,
		.vendor_name = "ESMT",
	},
	{
		.vendor_id = 0x13,
		.vendor_name = "cxmt",
	},
	{
		.vendor_id = 0x1A,
		.vendor_name = "Xi'an UniIC Semiconductors",
	},
	{
		.vendor_id = 0x1B,
		.vendor_name = "ISSI",
	},
	{
		.vendor_id = 0x1C,
		.vendor_name = "JSC",
	},
	{
		.vendor_id = 0xC5,
		.vendor_name = "SINKER",
	},
	{
		.vendor_id = 0xE5,
		.vendor_name = "Dosilicon Co,Ltd",
	},
	{
		.vendor_id = 0xF8,
		.vendor_name = "Fidelix",
	},
	{
		.vendor_id = 0xF9,
		.vendor_name = "Ultra Memory",
	},
	{
		.vendor_id = 0xFD,
		.vendor_name = "AP Memory",
	},
	{
		.vendor_id = 0xFF,
		.vendor_name = "Micron",
	},
	{
		.vendor_id = 0x00,
		.vendor_name = "unkown",
	},
};
const char *ddr_freq_array[] = {"unkown", "3200", "3733", "4266"};
const char *ddr_type_array[] = {"unkown", "lpddr4", "lpddr4x"};
const char *ddr_size_array[] = {"1G", "2G", "4G", "8G"};

EXPORT_SYMBOL_GPL(base_board_name);

#if 0
unsigned int x2_board_id[] = {
	0x100, 0x101, 0x102, 0x103, 0x200, 0x201, 0x202, 0x203, 0x204, 0x205,
	0x300, 0x301, 0x302, 0x303, 0x304, 0x400, 0x401, 0x104, 0x105, 0x106,
	0x107,
};

struct hobot_board_info board_of_id[] = {
	[0] = {X2_SVB, "X2-SVB"},
	[1] = {X2_DEV, "X2-DEV"},
	[2] = {X2SOM1V8, "X2SOM1V8"},
	[3] = {X2SOM3V3, "X2SOM3V3"},
	[4] = {J2_SVB, "J2-SVB"},
	[5] = {J2SOM, "J2SOM"},
	[6] = {J2_Mono, "J2-Mono"},
	[7] = {J2_DEV, "J2-DEV"},
	[8] = {J2_SK, "J2-SK"},
	[9] = {J2_SAM, "J2-SAM"},
	[10] = {J2_Quad, "J2-Quad*"},
	[11] = {J2_QuadJ2A, "J2-QuadJ2A"},
	[12] = {J2_QuadJ2B, "J2-QuadJ2B"},
	[13] = {J2_QuadJ2C, "J2-QuadJ2C"},
	[14] = {J2_QuadJ2D, "J2-QuadJ2D"},
	[15] = {J2_mm, "J2-mm"},
        [16] = {J2_mm_s202, "J2-mm-s202"},
	[17] = {X2_SOMFULL, "X2-SOMFULL"},
	[18] = {X2_96BOARD, "X2_96BOARD"},
	[19] = {X2_DEV512M, "X2-DEV512M"},
	[20] = {X2_XIAOMI, "X2-XIAOMI"},
	[21] = {Unknown, "Unknown"},
};

static int parse_boardid(uint32_t board_id)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(x2_board_id); i++) {
		if (board_id == x2_board_id[i])
			break;
	}

	if(i > ARRAY_SIZE(x2_board_id))
		i = ARRAY_SIZE(x2_board_id);
	return i;
}
#endif

ssize_t id_show(struct class *class,
			struct class_attribute *attr, char *buf)
{
	if (board_id == NULL)
		return 0;

	snprintf(buf, BUF_LEN, "%s", board_id);
	strcat(buf, "\n");

	return strlen(buf);
}

ssize_t origin_id_show(struct class *class,
			struct class_attribute *attr, char *buf)
{
	if (origin_board_id == NULL)
		return 0;

	snprintf(buf, BUF_LEN, "%s\n", origin_board_id);

	return strlen(buf);
}


ssize_t name_show(struct class *class,
			struct class_attribute *attr, char *buf)
{
	snprintf(buf, BUF_LEN, "%s-%s-%s-%s\n", base_board_name, ddr_vendor, ddr_size, ddr_freq);

	return strlen(buf);
}

ssize_t boot_show(struct class *class,
			struct class_attribute *attr, char *buf)
{
	if (!buf || !bootmode)
		return 0;
	snprintf(buf, BUF_LEN, "%s\n", bootmode);

	return strlen(buf);
}

ssize_t socuid_show(struct class *class,
			struct class_attribute *attr, char *buf)
{
	if (!buf)
		return 0;
	snprintf(buf, BUF_LEN, "%s\n", socuid);

	return strlen(buf);
}

ssize_t ddr_vender_show(struct class *class,
			struct class_attribute *attr, char *buf)
{
	if (!buf)
		return 0;
	snprintf(buf, BUF_LEN, "%s\n", ddr_vendor);

	return strlen(buf);
}

ssize_t ddr_name_show(struct class *class,
			struct class_attribute *attr, char *buf)
{
	if (!buf)
		return 0;
	snprintf(buf, BUF_LEN, "%s\n", ddr_type);

	return strlen(buf);
}

ssize_t ddr_freq_show(struct class *class,
			struct class_attribute *attr, char *buf)
{
	if (!buf)
		return 0;
	snprintf(buf, BUF_LEN, "%s\n", ddr_freq);

	return strlen(buf);
}

ssize_t ddr_size_show(struct class *class,
			struct class_attribute *attr, char *buf)
{
	if (!buf)
		return 0;
	snprintf(buf, BUF_LEN, "%s\n", ddr_size);

	return strlen(buf);
}

ssize_t ddr_part_num_show(struct class *class,
			struct class_attribute *attr, char *buf)
{
	if (!buf)
		return 0;
	snprintf(buf, BUF_LEN, "%s\n", ddr_part_num);

	return strlen(buf);
}

ssize_t som_name_show(struct class *class,
			struct class_attribute *attr, char *buf)
{
	if (!buf)
		return 0;
	snprintf(buf, BUF_LEN, "%s\n", som_name);

	return strlen(buf);
}

ssize_t base_board_name_show(struct class *class,
			struct class_attribute *attr, char *buf)
{
	if (!buf)
		return 0;
	snprintf(buf, BUF_LEN, "%s\n", base_board_name);

	return strlen(buf);
}

ssize_t chip_id_show(struct class *class,
			struct class_attribute *attr, char *buf)
{
	if (!buf)
		return 0;
	snprintf(buf, BUF_LEN, "0x%s\n", chip_id);

	return strlen(buf);
}

ssize_t secure_chip_show(struct class *class,
			struct class_attribute *attr, char *buf)
{
	if (!buf)
		return 0;
	snprintf(buf, BUF_LEN, "%d\n", secure_chip);

	return strlen(buf);
}

ssize_t bak_slot_show(struct class *class,
			struct class_attribute *attr, char *buf)
{
	uint32_t bak_slot = 0;
	if (!buf)
		return 0;
	bak_slot = (readl(g_bak_slot_info.map) & g_bak_slot_info.reg_info[3]) >> g_bak_slot_info.reg_info[2];
	snprintf(buf, BUF_LEN, "%d\n", bak_slot);

	return strlen(buf);
}

ssize_t soc_store(struct class *class, struct class_attribute *attr,
				const char *buf, size_t count)
{
	return count;
}

ssize_t bak_slot_store(struct class *class, struct class_attribute *attr,
				const char *buf, size_t count)
{
	uint32_t cur_boot_count = 0;
	uint32_t old_boot_count = 0;

	sscanf(buf, "%du", &cur_boot_count);
	old_boot_count = readl(g_boot_count.map) & ~g_boot_count.reg_info[3];
	cur_boot_count = old_boot_count | (cur_boot_count << g_boot_count.reg_info[2]);
	writel(cur_boot_count, g_boot_count.map);
	return count;
}

static struct class_attribute name_attribute =
	__ATTR(board_name, 0644, name_show, soc_store);

static struct class_attribute id_attribute =
	__ATTR(board_id, 0644, id_show, soc_store);

static struct class_attribute origin_id_attribute =
	__ATTR(origin_board_id, 0644, origin_id_show, soc_store);

static struct class_attribute ddr_vender_attribute =
	__ATTR(ddr_vendor, 0644, ddr_vender_show, soc_store);

static struct class_attribute ddr_name_attribute =
	__ATTR(ddr_type, 0644, ddr_name_show, soc_store);

static struct class_attribute ddr_freq_attribute =
	__ATTR(ddr_freq, 0644, ddr_freq_show, soc_store);

static struct class_attribute ddr_size_attribute =
	__ATTR(ddr_size, 0644, ddr_size_show, soc_store);

static struct class_attribute ddr_part_num_attribute =
	__ATTR(ddr_part_num, 0644, ddr_part_num_show, soc_store);

static struct class_attribute som_name_attribute =
	__ATTR(som_name, 0644, som_name_show, soc_store);

static struct class_attribute base_board_name_attribute =
	__ATTR(base_board_name, 0644, base_board_name_show, soc_store);

static struct class_attribute boot_attribute =
	__ATTR(boot_mode, 0644, boot_show, soc_store);

static struct class_attribute socuid_attribute =
	__ATTR(soc_uid, 0644, socuid_show, soc_store);

static struct class_attribute chip_id_attribute =
	__ATTR(chip_id, 0644, chip_id_show, soc_store);

static struct class_attribute secure_chip_attribute =
	__ATTR(secure_chip, 0444, secure_chip_show, NULL);

static struct class_attribute bak_slot_attribute =
	__ATTR(bak_slot, 0644, bak_slot_show, bak_slot_store);

static struct attribute *socinfo_attributes[] = {
	&name_attribute.attr,
	&id_attribute.attr,
	&origin_id_attribute.attr,
	&ddr_vender_attribute.attr,
	&ddr_name_attribute.attr,
	&ddr_freq_attribute.attr,
	&ddr_size_attribute.attr,
	&ddr_part_num_attribute.attr,
	&som_name_attribute.attr,
	&base_board_name_attribute.attr,
	&boot_attribute.attr,
	&socuid_attribute.attr,
	&chip_id_attribute.attr,
	&secure_chip_attribute.attr,
	&bak_slot_attribute.attr,
	NULL
};

static const struct attribute_group socinfo_group = {
	.attrs = socinfo_attributes,
};

static const struct attribute_group *socinfo_attr_group[] = {
	&socinfo_group,
	NULL,
};

static struct class socinfo_class = {
	.name = "socinfo",
	.class_groups = socinfo_attr_group,
};

/* Match table for of_platform binding */
static const struct of_device_id socinfo_of_match[] = {
	{ .compatible = "hobot,x5-socinfo", },
	{}
};
MODULE_DEVICE_TABLE(of, socinfo_of_match);

static int socinfo_probe(struct platform_device *pdev)
{
	int ret = 0, i;
	struct resource *resource = NULL;
	static void __iomem *ddr_info_addr = NULL;
	uint32_t ddr_info = 0;

	dev_info(&pdev->dev, "Start socinfo probe.\n");

	ret = of_property_read_string(pdev->dev.of_node, "board_name",
		&board_name);
	if (ret != 0) {
		pr_err("of_property_read_string error\n");
		return ret;
	}

	ret = of_property_read_string(pdev->dev.of_node, "board_id",
		&board_id);
	if (ret != 0) {
		pr_err("of_property_read_string error\n");
		return ret;
	}

	ret = of_property_read_string(pdev->dev.of_node, "origin_board_id",
		&origin_board_id);
	if (ret != 0) {
		pr_err("of_property_read_string error\n");
		return ret;
	}

	ret = of_property_read_string(pdev->dev.of_node, "som_name",
		&som_name);
	if (ret != 0) {
		pr_err("of_property_read_string error\n");
		return ret;
	}

	ret = of_property_read_string(pdev->dev.of_node, "base_board_name",
		&base_board_name);
	if (ret != 0) {
		pr_err("of_property_read_string error\n");
		return ret;
	}

	ret = of_property_read_string(pdev->dev.of_node, "boot_mode",
		&bootmode);
	if (ret != 0) {
		pr_err("of_property_read_string error\n");
		return ret;
	}

	ret = of_property_read_string(pdev->dev.of_node, "socuid", &socuid);
	if (ret != 0) {
		pr_err("of_property_read_string error\n");
		return ret;
	}

	ret = of_property_read_string(pdev->dev.of_node, "chip_id", &chip_id);
	if (ret != 0) {
		pr_err("of_property_read_string error\n");
		return ret;
	}

	ret = of_property_read_u32_array(pdev->dev.of_node,
					 "bak_slot_reg",
					 g_bak_slot_info.reg_info,
					 ARRAY_SIZE(g_bak_slot_info.reg_info));
	if (ret != 0) {
		pr_err("of_property_read_u32_array error\n");
		return ret;
	}

	g_bak_slot_info.map = ioremap(g_bak_slot_info.reg_info[0],
					 g_bak_slot_info.reg_info[1]);
	if (!g_bak_slot_info.map)
		return -ENOMEM;

	ret = of_property_read_u32_array(pdev->dev.of_node,
					 "boot_count_reg",
					 g_boot_count.reg_info,
					 ARRAY_SIZE(g_boot_count.reg_info));
	if (ret != 0) {
		pr_err("of_property_read_u32_array error\n");
		return ret;
	}

	g_boot_count.map = ioremap(g_boot_count.reg_info[0],
					 g_boot_count.reg_info[1]);
	if (!g_boot_count.map)
		return -ENOMEM;

	resource = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (resource == NULL) {
		dev_err(&pdev->dev, "Can't get ddr info resource error\n");
		return -ENODEV;
	}

	ddr_info_addr = devm_ioremap_resource(&pdev->dev, resource);
	if (IS_ERR(ddr_info_addr)) {
		dev_err(&pdev->dev, "Can't get ddr info resource\n");
		return (int32_t)PTR_ERR(ddr_info_addr);
	}
	ddr_info = readl(ddr_info_addr);
	ddr_freq = ddr_freq_array[DR_DDR_FREQ(ddr_info)];
	ddr_size = ddr_size_array[DR_DDR_SIZE(ddr_info)];
	ddr_type = ddr_type_array[DR_DDR_TYPE(ddr_info)];
	for (i = 0; i < ARRAY_SIZE(ddr_vendor_info); i++) {
		if (DR_DDR_VENDOR(ddr_info) == ddr_vendor_info[i].vendor_id ||
			i == ARRAY_SIZE(ddr_vendor_info) - 1) {
			ddr_vendor = ddr_vendor_info[i].vendor_name;
			break;
		}
	}

	ret = class_register(&socinfo_class);
	dev_info(&pdev->dev, "Socinfo probe end with retval: %d\n", ret);

	if (ret < 0)
		return ret;

	return 0;
}

static int socinfo_remove(struct platform_device *pdev)
{
	iounmap(sec_flag_addr);
	iounmap(g_boot_count.map);
	iounmap(g_bak_slot_info.map);
	class_unregister(&socinfo_class);
	return 0;
}

static struct platform_driver socinfo_platform_driver = {
	.probe   = socinfo_probe,
	.remove  = socinfo_remove,
	.driver  = {
		.name = SOCINFO_NAME,
		.of_match_table = socinfo_of_match,
		},
};

static int __init socinfo_init(void)
{
	int retval = 0;
	pr_info("Start socinfo driver init\n");
	/* Register the platform driver */
	retval = platform_driver_register(&socinfo_platform_driver);
	if (retval)
		pr_err("Unable to register platform driver\n");

	return retval;
}

static void __exit socinfo_exit(void)
{
	/* Unregister the platform driver */
	platform_driver_unregister(&socinfo_platform_driver);

}

module_init(socinfo_init);
module_exit(socinfo_exit);

MODULE_DESCRIPTION("Driver for SOCINFO");
MODULE_AUTHOR("Horizon Inc.");
MODULE_LICENSE("GPL");
