/*
 *  Ubiquiti RouterStation support
 *
 *  Copyright (C) 2008 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *  Copyright (C) 2008 Ubiquiti <support@ubnt.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/input.h>

#include <asm/mips_machine.h>
#include <asm/mach-ar71xx/ar71xx.h>
#include <asm/mach-ar71xx/pci.h>
#include <asm/mach-ar71xx/platform.h>

#define UBNT_RS_GPIO_LED_RF	2
#define UBNT_RS_GPIO_SW4	8

#define UBNT_BUTTONS_POLL_INTERVAL	20

static struct spi_board_info ubnt_spi_info[] = {
	{
		.bus_num	= 0,
		.chip_select	= 0,
		.max_speed_hz	= 25000000,
		.modalias	= "m25p80",
	}
};

static struct ar71xx_pci_irq ubnt_rs_pci_irqs[] __initdata = {
	{
		.slot	= 0,
		.pin	= 1,
		.irq	= AR71XX_PCI_IRQ_DEV0,
	}, {
		.slot	= 1,
		.pin	= 1,
		.irq	= AR71XX_PCI_IRQ_DEV1,
	}, {
		.slot	= 2,
		.pin	= 1,
		.irq	= AR71XX_PCI_IRQ_DEV2,
	}
};

static struct gpio_led ubnt_rs_leds_gpio[] __initdata = {
	{
		.name		= "ubnt:green:rf",
		.gpio		= UBNT_RS_GPIO_LED_RF,
		.active_low	= 0,
	}
};

static struct gpio_button ubnt_rs_gpio_buttons[] __initdata = {
	{
		.desc		= "sw4",
		.type		= EV_KEY,
		.code		= BTN_0,
		.threshold	= 5,
		.gpio		= UBNT_RS_GPIO_SW4,
		.active_low	= 1,
	}
};

#define UBNT_RS_WAN_PHYMASK	(1 << 20)
#define UBNT_RS_LAN_PHYMASK	((1 << 16) | (1 << 17) | (1 << 18) | (1 << 19))

static void __init ubnt_rs_setup(void)
{
	ar71xx_add_device_spi(NULL, ubnt_spi_info,
				    ARRAY_SIZE(ubnt_spi_info));

	ar71xx_add_device_mdio(~(UBNT_RS_WAN_PHYMASK | UBNT_RS_LAN_PHYMASK));

	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ar71xx_eth0_data.phy_mask = UBNT_RS_WAN_PHYMASK;

	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ar71xx_eth1_data.phy_mask = UBNT_RS_LAN_PHYMASK;

	ar71xx_eth1_data.speed = SPEED_100;
	ar71xx_eth1_data.duplex = DUPLEX_FULL;

	ar71xx_add_device_eth(0);
	ar71xx_add_device_eth(1);

	ar71xx_add_device_usb();

	ar71xx_pci_init(ARRAY_SIZE(ubnt_rs_pci_irqs), ubnt_rs_pci_irqs);

	ar71xx_add_device_leds_gpio(-1, ARRAY_SIZE(ubnt_rs_leds_gpio),
					ubnt_rs_leds_gpio);

	ar71xx_add_device_gpio_buttons(-1, UBNT_BUTTONS_POLL_INTERVAL,
					ARRAY_SIZE(ubnt_rs_gpio_buttons),
					ubnt_rs_gpio_buttons);
}

MIPS_MACHINE(AR71XX_MACH_UBNT_RS, "Ubiquiti RouterStation", ubnt_rs_setup);

static void __init ubnt_lsx_setup(void)
{
	ar71xx_add_device_spi(NULL, ubnt_spi_info,
				    ARRAY_SIZE(ubnt_spi_info));
}

MIPS_MACHINE(AR71XX_MACH_UBNT_LSX, "Ubiquiti LSX", ubnt_lsx_setup);
