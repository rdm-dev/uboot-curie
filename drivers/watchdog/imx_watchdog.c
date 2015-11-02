/*
 * watchdog.c - driver for i.mx on-chip watchdog
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <asm/io.h>
#include <watchdog.h>
#include <asm/arch/imx-regs.h>

struct watchdog_regs {
	u16	wcr;	/* Control */
	u16	wsr;	/* Service */
	u16	wrsr;	/* Reset Status */
};

#define WCR_WDZST	0x01
#define WCR_WDBG	0x02
#define WCR_WDE		0x04	/* WDOG enable */
#define WCR_WDT		0x08
#define WCR_SRS		0x10
#define SET_WCR_WT(x)	(x << 8)

#ifdef CONFIG_IMX_WATCHDOG
void hw_watchdog_reset(void)
{
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;

	writew(0x5555, &wdog->wsr);
	writew(0xaaaa, &wdog->wsr);
}

void hw_watchdog_init(void)
{
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;
	u16 timeout;

	/*
	 * The timer watchdog can be set between
	 * 0.5 and 128 Seconds. If not defined
	 * in configuration file, sets 128 Seconds
	 */
#ifndef CONFIG_WATCHDOG_TIMEOUT_MSECS
#define CONFIG_WATCHDOG_TIMEOUT_MSECS 128000
#endif
	timeout = (CONFIG_WATCHDOG_TIMEOUT_MSECS / 500) - 1;
	writew(WCR_WDZST | WCR_WDBG | WCR_WDE | WCR_WDT | WCR_SRS |
		SET_WCR_WT(timeout), &wdog->wcr);
	hw_watchdog_reset();
}
#endif

void reset_cpu(ulong addr)
{
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;

	/* Workaround for curie reboot issue
	 * see linux/curie_4.1: cef5f68ac5e1fa068bfdecd20a8b0ac14c929aa9
	 */

	/* To change the bootcfg by software for curie board:
	   1. load required bootcfg to SRC_GPR9 (0x020d8040)
	   2. set bit 28 of SRC_GPR10 (0x020d8044)
	   3. then reset the system

	   to return to normal boot mode, clear SRC_GPR10[28] */
	// eMMC 1-bit mode
	do {
#define	SRC_BASE_ADDR_BMSR1		SRC_BASE_ADDR + 0x04
#define	SRC_BASE_ADDR_GPR9		SRC_BASE_ADDR + 0x40
#define	SRC_BASE_ADDR_GPR10		SRC_BASE_ADDR + 0x44

		u32 bmsr1 = __raw_readl(SRC_BASE_ADDR_BMSR1);
		//u32 gpr9 = __raw_readl(SRC_BASE_ADDR_GPR9);
		u32 gpr10 = __raw_readl(SRC_BASE_ADDR_GPR10);

		if (bmsr1 == 0x4000d860) {
			// original mode is eMMC 8-bit DDR boot
			writel(		0x40001860,	SRC_BASE_ADDR_GPR9);
			writel(gpr10 |	0x10000000,	SRC_BASE_ADDR_GPR10);
		} else {
			// original mode is SD boot, unchanged
		}
	} while(0);

#if defined(CONFIG_MX7)
	writew((WCR_WDE | WCR_SRS), &wdog->wcr);
#else
	writew(WCR_WDE, &wdog->wcr);
#endif
	writew(0x5555, &wdog->wsr);
	writew(0xaaaa, &wdog->wsr);	/* load minimum 1/2 second timeout */
	while (1) {
		/*
		 * spin for .5 seconds before reset
		 */
	}
}
