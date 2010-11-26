include $(TOPDIR)/rules.mk

PKG_NAME:=nodewatcher
PKG_VERSION:=0.2
PKG_RELEASE:=1

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
  # Base files
	$(INSTALL_DIR) $(1)/www/cgi-bin
	$(INSTALL_BIN) ./files/nodewatcher $(1)/www/cgi-bin
	$(INSTALL_BIN) ./files/opkgwatcher $(1)/www/cgi-bin
	
	# Nodogsplash output parser
	# TODO move to nodewatcher-nodogsplash package
	$(INSTALL_DIR) $(1)/etc/nodewatcher
	$(CP) ./files/parse_nds.awk $(1)/etc/nodewatcher
	
	# Sanity checks
	$(INSTALL_DIR) $(1)/etc/crontabs/run.5
	$(INSTALL_BIN) ./files/check_sanity $(1)/etc/crontabs/run.5/00-check_sanity
	$(INSTALL_BIN) ./files/check_wifi_channel $(1)/etc/crontabs/run.5/01-check_wifi_channel
	$(INSTALL_BIN) ./files/check_dns $(1)/etc/crontabs/run.5/02-check_dns
	$(INSTALL_BIN) ./files/count_wifi_errors $(1)/etc/crontabs/run.5/03-count_wifi_errors
	
	# Nullhttpd templates
	$(INSTALL_DIR) $(1)/www/nullhttpd
	$(CP) -r ./files/htdocs/* $(1)/www/nullhttpd
	
	# Actions
	$(INSTALL_DIR) $(1)/etc/actions.d
	$(INSTALL_BIN) ./files/olsr_gateway_action $(1)/etc/actions.d/olsr_gateway_action
	$(INSTALL_BIN) ./files/olsr_dns_action $(1)/etc/actions.d/olsr_dns_action
	
	# Common functions
	$(INSTALL_DIR) $(1)/lib/nodewatcher
	$(CP) ./files/actions_common.sh $(1)/lib/nodewatcher/actions.sh
	$(CP) ./files/nodewatcher_common.sh $(1)/lib/nodewatcher/common.sh
	
	# Modules
	$(INSTALL_DIR) $(1)/etc/nodewatcher.d
	$(CP) ./files/modules/general.sh $(1)/etc/nodewatcher.d/general.sh
	$(CP) ./files/modules/wireless.sh $(1)/etc/nodewatcher.d/wireless.sh
	$(CP) ./files/modules/nettests.sh $(1)/etc/nodewatcher.d/nettests.sh
	$(CP) ./files/modules/vpn.sh $(1)/etc/nodewatcher.d/vpn.sh
	$(CP) ./files/modules/traffic.sh $(1)/etc/nodewatcher.d/traffic.sh
	$(CP) ./files/modules/dhcp.sh $(1)/etc/nodewatcher.d/dhcp.sh
	# TODO move to nodewatcher-nodogsplash package
	$(CP) ./files/modules/nodogsplash.sh $(1)/etc/nodewatcher.d/nodogsplash.sh
	# TODO move to nodewatcher-solar package
	$(CP) ./files/modules/solar.sh $(1)/etc/nodewatcher.d/solar.sh
	# TODO move to nodewatcher-digitemp package
	$(CP) ./files/modules/digitemp.sh $(1)/etc/nodewatcher.d/digitemp.sh
endef

$(eval $(call BuildPackage,nodewatcher))
