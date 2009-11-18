include $(TOPDIR)/rules.mk

PKG_NAME:=nullhttpd
PKG_VERSION:=0.1
PKG_RELEASE:=4

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
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/nullhttpd.init $(1)/etc/init.d/nullhttpd
endef

$(eval $(call BuildPackage,nullhttpd))
