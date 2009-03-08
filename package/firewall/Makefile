#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=firewall

PKG_VERSION:=1
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define Package/firewall
  SECTION:=net
  CATEGORY:=Base system
  URL:=http://openwrt.org/
  TITLE:=OpenWrt firewall
  DEPENDS:=+iptables-mod-conntrack +iptables-mod-nat
endef

define Package/firewall/description
 uci based firewall for openwrt
endef

define Build/Compile
	true
endef

define Package/firewall/install
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/firewall.init $(1)/etc/init.d/firewall
endef

$(eval $(call BuildPackage,firewall))
