#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 12175 2008-08-05 22:04:10Z florian $

include $(TOPDIR)/rules.mk

PKG_NAME:=openvpn
PKG_VERSION:=2.0.9
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://openvpn.net/release @SF/openvpn
PKG_MD5SUM:=60745008b90b7dbe25fe8337c550fec6

include $(INCLUDE_DIR)/package.mk

define Package/openvpn
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+kmod-tun +libopenssl +liblzo
  TITLE:=Open source VPN solution using SSL
  URL:=http://openvpn.net
  SUBMENU:=VPN
endef

define Package/openvpn/description
	 Open source VPN solution using SSL
endef

define Package/openvpn-easy-rsa
  $(call Package/openvpn)
  DEPENDS:=openvpn +openssl-util
  TITLE:=Simple shell scripts to manage a Certificate Authority
endef

define Build/Configure
	$(call Build/Configure/Default, \
		--disable-pthread \
		--disable-debug \
		--disable-plugins \
		--enable-management \
		--disable-socks \
		--enable-password-save \
		,\
		ac_cv_func_epoll_create=no \
	)
endef

define Build/Compile	
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install
endef

define Package/openvpn/install	
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/openvpn $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/etc/default
	$(INSTALL_BIN) files/$(PKG_NAME).default $(1)/etc/default/$(PKG_NAME)
endef

define Package/openvpn-easy-rsa/install	
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/easy-rsa/2.0/{build-*,clean-all,inherit-inter,list-crl,pkitool,revoke-full,sign-req,whichopensslcnf} $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/easy-rsa
	install -m 0644 $(PKG_BUILD_DIR)/easy-rsa/2.0/openssl.cnf $(1)/etc/easy-rsa/openssl.cnf
	install -m 0644 $(PKG_BUILD_DIR)/easy-rsa/2.0/vars $(1)/etc/easy-rsa/vars
endef

$(eval $(call BuildPackage,openvpn))
$(eval $(call BuildPackage,openvpn-easy-rsa))
