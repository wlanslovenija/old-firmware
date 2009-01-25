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
PKG_RELEASE:=7

include $(INCLUDE_DIR)/package.mk

define Package/solar
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+cronscripts
  TITLE:=Solar Manager
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Package/solar/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/solar $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/crontabs/run.30
	$(INSTALL_BIN) ./files/push-status.sh $(1)/etc/crontabs/run.30/solar
endef

$(eval $(call BuildPackage,solar))
