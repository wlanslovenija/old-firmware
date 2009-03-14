# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 10138 2008-01-06 19:28:26Z nbd $

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=kmod-switch
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

ifeq ($(BOARD),brcm-2.4)
  BUILDFLAGS := -DBROADCOM -DBCMDRIVER -I$(LINUX_DIR)/arch/mips/bcm947xx/include
  ADM_SWITCH := 1
else
  ifeq ($(BOARD),brcm47xx)
    BUILDFLAGS := -DBROADCOM
    ADM_SWITCH := 1
  else
    ADM_SWITCH := 0
  endif
endif

define KernelPackage/switch
  SUBMENU:=Other modules
  TITLE:=Switch drivers
  FILES:= \
	$(PKG_BUILD_DIR)/switch-core.$(LINUX_KMOD_SUFFIX) \
	$(PKG_BUILD_DIR)/switch-robo.$(LINUX_KMOD_SUFFIX)

  ifeq ($(ADM_SWITCH),1)
    AUTOLOAD:=$(call AutoLoad,20,switch-core switch-robo switch-adm)
    FILES += $(PKG_BUILD_DIR)/switch-adm.$(LINUX_KMOD_SUFFIX)
  else
    AUTOLOAD:=$(call AutoLoad,20,switch-core switch-robo switch-ip175c)
    FILES += $(PKG_BUILD_DIR)/switch-ip175c.$(LINUX_KMOD_SUFFIX)
  endif
endef

define KernelPackage/switch/description
 This package contains switch drivers for ADM6996L and BCM53XX RoboSwitch.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(BUILDFLAGS)" \
		ADM_SWITCH="$(ADM_SWITCH)" \
		modules
endef

define Build/InstallDev
	mkdir -p $(1)/usr/include
	$(CP) ./src/switch-core.h $(1)/usr/include/
endef

define Build/UninstallDev
	rm -f $(1)/usr/include/switch-core.h
endef

define KernelPackage/switch/install
	$(INSTALL_DIR) $(1)/lib/network/
	$(INSTALL_BIN) ./files/switch.sh $(1)/lib/network/
endef

$(eval $(call KernelPackage,switch))
