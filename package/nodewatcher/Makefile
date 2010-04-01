include $(TOPDIR)/rules.mk

PKG_NAME:=nodewatcher
PKG_VERSION:=0.1
PKG_RELEASE:=26

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
	$(INSTALL_DIR) $(1)/etc/crontabs/run.5
	$(INSTALL_BIN) ./files/check_wifi_channel $(1)/etc/crontabs/run.5/01-check_wifi_channel
	$(INSTALL_BIN) ./files/check_sanity $(1)/etc/crontabs/run.5/00-check_sanity
	$(INSTALL_BIN) ./files/check_dns $(1)/etc/crontabs/run.5/02-check_dns
	$(INSTALL_DIR) $(1)/www/nullhttpd
	$(CP) -r ./files/htdocs/* $(1)/www/nullhttpd
	$(INSTALL_DIR) $(1)/etc/actions.d
	$(INSTALL_BIN) ./files/olsr_gateway_action $(1)/etc/actions.d/olsr_gateway_action
	$(INSTALL_BIN) ./files/olsr_dns_action $(1)/etc/actions.d/olsr_dns_action
	$(CP) ./files/actions_common.sh $(1)/etc/actions.d/actions_common.sh
endef

$(eval $(call BuildPackage,nodewatcher))
