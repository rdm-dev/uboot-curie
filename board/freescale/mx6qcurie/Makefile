#
# Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
#
# (C) Copyright 2011-2013 Freescale Semiconductor, Inc.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB    = $(obj)lib$(BOARD).o

COBJS  := mx6qcurie.o

SRCS   := $(COBJS:.o=.c)
OBJS   := $(addprefix $(obj),$(COBJS))

all:	$(LIB) plugin.bin

$(LIB):        $(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))


# plugin entry
PLUGIN_OBJS = plugin/plugin_entry.o
# plugin C main function
PLUGIN_OBJS += plugin/plugin_main.o plugin/plugin_data.o plugin/crc32.o
# plugin env
PLUGIN_OBJS += plugin/_divsi3.o plugin/_udivsi3.o plugin/div0.o plugin/cortexA9.o plugin/armv7_cache.o
# plugin drivers
PLUGIN_OBJS += plugin/ccm_pll.o
PLUGIN_OBJS += plugin/hardware_modules.o
PLUGIN_OBJS += plugin/uart_iomux_config.o plugin/uart1_iomux_config.o
PLUGIN_OBJS += plugin/usdhc_iomux_config.o plugin/usdhc4_iomux_config.o plugin/board_usdhc.o
PLUGIN_OBJS += plugin/timer.o plugin/epit.o
PLUGIN_OBJS += plugin/imx_uart.o
PLUGIN_OBJS += plugin/usdhc.o plugin/usdhc_host.o plugin/usdhc_mmc.o
# plugin libs
PLUGIN_OBJS += plugin/xprintf.o
# plugin asm code
PLUGIN_OBJS += plugin/plugin.o

plugin.o : plugin.S plugin_common.S

plugin.bin: plugin/plugin.elf
	$(OBJCOPY) -O binary --gap-fill 0xff $< $@

plugin/plugin.elf: $(PLUGIN_OBJS)
	$(LD) -T plugin/plugin.lds -Bstatic -nostartfiles -Map plugin/plugin.map -o $@ $(PLUGIN_OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
