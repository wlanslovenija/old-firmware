# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 7742 2007-06-27 05:26:20Z nbd $

include $(TOPDIR)/rules.mk

PKG_NAME:=nullhttpd
PKG_VERSION:=0.1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/nullhttpd
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+nodogsplash
  TITLE:=Null HTTP daemon
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Package/nullhttpd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/nullhttpd $(1)/usr/bin/
endef

$(eval $(call BuildPackage,nullhttpd))
