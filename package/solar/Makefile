include $(TOPDIR)/rules.mk

PKG_NAME:=solar
PKG_VERSION:=0.1
PKG_RELEASE:=11

include $(INCLUDE_DIR)/package.mk

define Package/solar
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+nodewatcher
  TITLE:=Solar Manager
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Package/solar/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/solar $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/solar.init $(1)/etc/init.d/solar
	$(INSTALL_DIR) $(1)/etc/crontabs/run.30
	$(INSTALL_BIN) ./files/solar_measure $(1)/etc/crontabs/run.30
endef

$(eval $(call BuildPackage,solar))
