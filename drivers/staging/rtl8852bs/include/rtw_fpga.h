/******************************************************************************
 *
 * Copyright(c) 2007 - 2021 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#ifndef __RTW_FPGA_H_
#define __RTW_FPGA_H_


#if defined(CONFIG_FPGA_INCLUDED)
void rtw_fpga_start(_adapter *padapter);
void rtw_fpga_stop(_adapter *padapter);
#endif /* CONFIG_FPGA_INCLUDED */

#endif
