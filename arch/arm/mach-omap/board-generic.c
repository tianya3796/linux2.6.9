/*
 * linux/arch/arm/mach-omap/board-generic.c
 *
 * Modified from board-innovator1510.c
 *
 * Code for generic OMAP board. Should work on many OMAP systems where
 * the device drivers take care of all the necessary hardware initialization.
 * Do not put any board specific code to this file; create a new machine
 * type if you need custom low-level initializations.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>

#include <asm/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <asm/arch/clocks.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mux.h>
#include <asm/arch/usb.h>
#include <asm/arch/board.h>

#include "common.h"

extern void __init omap_init_time(void);

static void __init omap_generic_init_irq(void)
{
	omap_init_irq();
}

/*
 * Muxes the serial ports on
 */
#ifdef CONFIG_ARCH_OMAP1510
static void __init omap_early_serial_init(void)
{
#ifdef CONFIG_OMAP_LL_DEBUG_UART1
	omap_cfg_reg(UART1_TX);
	omap_cfg_reg(UART1_RTS);
#endif

#ifdef CONFIG_OMAP_LL_DEBUG_UART2
	omap_cfg_reg(UART2_TX);
	omap_cfg_reg(UART2_RTS);
#endif

#ifdef CONFIG_OMAP_LL_DEBUG_UART1
	omap_cfg_reg(UART3_TX);
	omap_cfg_reg(UART3_RX);
#endif
}
#endif

/* assume no Mini-AB port */

#ifdef CONFIG_ARCH_OMAP1510
static struct omap_usb_config generic1510_usb_config __initdata = {
	.register_host	= 1,
	.register_dev	= 1,
	.hmc_mode	= 16,
	.pins[0]	= 3,
};
#endif

#ifdef CONFIG_ARCH_OMAP1610
static struct omap_usb_config generic1610_usb_config __initdata = {
	.register_host	= 1,
	.register_dev	= 1,
	.hmc_mode	= 16,
	.pins[0]	= 6,
};
#endif

static struct omap_board_config_kernel generic_config[] = {
	{ OMAP_TAG_USB,           NULL },
};

static void __init omap_generic_init(void)
{
	/*
	 * Make sure the serial ports are muxed on at this point.
	 * You have to mux them off in device drivers later on
	 * if not needed.
	 */
#ifdef CONFIG_ARCH_OMAP1510
	if (cpu_is_omap1510()) {
		omap_early_serial_init();
		generic_config[0].data = &generic1510_usb_config;
	}
#endif
#ifdef CONFIG_ARCH_OMAP1610
	if (!cpu_is_omap1510()) {
		generic_config[0].data = &generic1610_usb_config;
	}
#endif
	omap_board_config = generic_config;
	omap_board_config_size = ARRAY_SIZE(generic_config);
}

static void __init omap_generic_map_io(void)
{
	omap_map_io();
}

MACHINE_START(OMAP_GENERIC, "Generic OMAP1510/1610/1710")
	MAINTAINER("Tony Lindgren <tony@atomide.com>")
	BOOT_MEM(0x10000000, 0xfff00000, 0xfef00000)
	BOOT_PARAMS(0x10000100)
	MAPIO(omap_generic_map_io)
	INITIRQ(omap_generic_init_irq)
	INIT_MACHINE(omap_generic_init)
	INITTIME(omap_init_time)
MACHINE_END
