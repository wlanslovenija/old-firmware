# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 6501 2007-03-04 04:41:46Z pavlov $

include $(TOPDIR)/rules.mk

PKG_NAME:=messyd
PKG_VERSION:=0.01
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/messyd

include $(INCLUDE_DIR)/package.mk

define Package/messyd
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Mesh management daemon (messyd)
  DESCRIPTION:=\
	This package contains a daemon for managing some important things like \\\
	automagic neighbour router registration.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/messyd/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/messyd $(1)/sbin/
endef

$(eval $(call BuildPackage,messyd))
