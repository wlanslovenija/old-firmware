# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 7742 2007-06-27 05:26:20Z nbd $

include $(TOPDIR)/rules.mk

PKG_NAME:=nodewatcher
PKG_VERSION:=0.1
PKG_RELEASE:=16

include $(INCLUDE_DIR)/package.mk

define Package/nodewatcher
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+busybox +cronscripts
  TITLE:=NodeWatcher
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/nodewatcher/install
	$(INSTALL_DIR) $(1)/www/cgi-bin
	$(INSTALL_BIN) ./files/nodewatcher $(1)/www/cgi-bin
	$(INSTALL_BIN) ./files/opkgwatcher $(1)/www/cgi-bin
	$(INSTALL_DIR) $(1)/etc/nodewatcher
	$(CP) ./files/parse_nds.awk $(1)/etc/nodewatcher
	$(INSTALL_DIR) $(1)/etc/crontabs/run.30
	$(INSTALL_BIN) ./files/solar_measure $(1)/etc/crontabs/run.30
	$(INSTALL_DIR) $(1)/etc/crontabs/run.5
	$(INSTALL_BIN) ./files/check_wifi_channel $(1)/etc/crontabs/run.5
	$(INSTALL_BIN) ./files/check_sanity $(1)/etc/crontabs/run.5
	$(INSTALL_BIN) ./files/check_dns $(1)/etc/crontabs/run.5
endef

$(eval $(call BuildPackage,nodewatcher))
