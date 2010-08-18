#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 9695 2007-12-09 18:59:01Z nbd $

include $(TOPDIR)/rules.mk

PKG_NAME:=splash-slovenija
PKG_VERSION:=1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/splash-slovenija
  SUBMENU:=Captive Portals
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+nodogsplash
  TITLE:=Splash screen for wlan slovenija
  URL:=http://wlan-si.net
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/splash-slovenija/install
	$(INSTALL_DIR) $(1)/etc/nodogsplash
	$(INSTALL_DIR) $(1)/etc/nodogsplash/htdocs
	$(INSTALL_DIR) $(1)/etc/nodogsplash/htdocs/pages
	$(INSTALL_DIR) $(1)/etc/nodogsplash/htdocs/images
	$(CP) ./files/*.html $(1)/etc/nodogsplash/htdocs/
	$(CP) ./files/\$$$$pagesdir/*.css $(1)/etc/nodogsplash/htdocs/pages
	$(CP) ./files/images/*.png $(1)/etc/nodogsplash/htdocs/images
endef

$(eval $(call BuildPackage,splash-slovenija))

