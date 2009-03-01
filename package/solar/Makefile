# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 7742 2007-06-27 05:26:20Z nbd $

include $(TOPDIR)/rules.mk

PKG_NAME:=solar
PKG_VERSION:=0.1
PKG_RELEASE:=10

include $(INCLUDE_DIR)/package.mk

define Package/solar
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=
  TITLE:=Solar Manager
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Package/solar/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/solar $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/solar.init $(1)/etc/init.d/solar
endef

$(eval $(call BuildPackage,solar))
