#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 9695 2007-12-09 18:59:01Z nbd $

include $(TOPDIR)/rules.mk

PKG_NAME:=cronscripts
PKG_VERSION:=0.3
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/cronscripts
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+busybox
  TITLE:=Cron scripts
  URL:=
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/cronscripts/install
	$(INSTALL_DIR) $(1)/etc/crontabs
	$(CP) ./files/main.crontab $(1)/etc/crontabs/root
	$(INSTALL_DIR) $(1)/etc/crontabs/run.5
	$(INSTALL_DIR) $(1)/etc/crontabs/run.10
	$(INSTALL_DIR) $(1)/etc/crontabs/run.30
	$(INSTALL_DIR) $(1)/etc/crontabs/run.hourly
	$(INSTALL_DIR) $(1)/etc/crontabs/run.daily
	$(INSTALL_DIR) $(1)/etc/crontabs/run.weekly
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/run-crons $(1)/usr/sbin/run-crons
endef

$(eval $(call BuildPackage,cronscripts))
