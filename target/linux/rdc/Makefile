#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=i386
BOARD:=rdc
BOARDNAME:=RDC 321x
FEATURES:=squashfs jffs2 pci usb

LINUX_VERSION:=2.6.24.7

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += hostapd-mini

define Target/Description
        Build firmware images for RDC321x based routers
        (e.g. Airlink101 AR525W, Linksys WRT54GR, Sitecom WL-153)
endef

$(eval $(call BuildTarget))
