#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk

PKG_NAME:=uClibc
PKG_VERSION:=$(call qstrip,$(CONFIG_UCLIBC_VERSION))
PKG_EXTRAVERSION:=$(call qstrip,$(CONFIG_UCLIBC_EXTRA_VERSION))
PKG_VERSION_SNAPSHOT:=$(call qstrip,$(CONFIG_UCLIBC_VERSION_SNAPSHOT))
ifeq ($(PKG_VERSION_SNAPSHOT),y)
  PKG_SOURCE_URL:=http://www.uclibc.org/downloads/snapshots
  PKG_SOURCE:=uClibc-snapshot.tar.bz2
else
  PKG_SOURCE_URL:=http://www.uclibc.org/downloads
  PKG_SOURCE:=uClibc-$(PKG_VERSION)$(PKG_EXTRAVERSION).tar.bz2
endif
PATCH_DIR:=./patches/$(PKG_VERSION)$(PKG_EXTRAVERSION)
CONFIG_DIR:=./config/$(PKG_VERSION)$(PKG_EXTRAVERSION)

ifeq ($(PKG_VERSION),0.9.28)
  ifeq ($(PKG_EXTRAVERSION),.2)
    PKG_MD5SUM:=959f25286e317f0d9e2103445c5a14c2
  endif
  ifeq ($(PKG_EXTRAVERSION),.3)
    PKG_MD5SUM:=428405a36b4662980d9343b32089b5a6
  endif
endif

ifeq ($(PKG_VERSION),0.9.29)
  ifeq ($(PKG_EXTRAVERSION),)
    # currently the default version, no special sub-directory needed
    PATCH_DIR:=./patches
    CONFIG_DIR:=./config
    PKG_MD5SUM:=61dc55f43b17a38a074f347e74095b20
  endif
endif

ifeq ($(PKG_VERSION),0.9.30)
  ifeq ($(PKG_EXTRAVERSION),)
    PKG_MD5SUM:=e5766e2566e0297adebebbcc0aba1f2d
  endif
endif

ifeq ($(PKG_VERSION_SNAPSHOT),y)
  PATCH_DIR:=
  CONFIG_DIR:=./config
  PKG_MD5SUM:=
endif

PKG_CAT:=bzcat

STAGING_DIR_HOST:=$(TOOLCHAIN_DIR)
BUILD_DIR_HOST:=$(BUILD_DIR_TOOLCHAIN)
ifeq ($(PKG_VERSION_SNAPSHOT),y)
  PKG_BUILD_DIR:=$(BUILD_DIR_HOST)/uClibc-$(PKG_EXTRAVERSION)
else
  PKG_BUILD_DIR:=$(BUILD_DIR_HOST)/uClibc-$(PKG_VERSION)$(PKG_EXTRAVERSION)
endif

override CONFIG_AUTOREBUILD=

include $(INCLUDE_DIR)/host-build.mk

STAMP_BUILT:=$(TOOLCHAIN_DIR)/stamp/.uclibc_installed
STAMP_INSTALLED:=$(TOOLCHAIN_DIR)/stamp/.uclibc-utils_installed

UCLIBC_TARGET_ARCH:=$(shell echo $(ARCH) | sed -e s'/-.*//' \
		-e 's/i.86/i386/' \
		-e 's/sparc.*/sparc/' \
		-e 's/arm.*/arm/g' \
		-e 's/avr32.*/avr32/g' \
		-e 's/m68k.*/m68k/' \
		-e 's/ppc/powerpc/g' \
		-e 's/v850.*/v850/g' \
		-e 's/sh64/sh/' \
		-e 's/sh[234].*/sh/' \
		-e 's/mips.*/mips/' \
		-e 's/mipsel.*/mips/' \
		-e 's/cris.*/cris/' \
)


define Build/SetToolchainInfo
	$(SED) 's,^\(LIBC_TYPE\)=.*,\1=$(PKG_NAME),' $(TOOLCHAIN_DIR)/info.mk
	$(SED) 's,^\(LIBC_URL\)=.*,\1=http://www.uclibc.org/,' $(TOOLCHAIN_DIR)/info.mk
	$(SED) 's,^\(LIBC_VERSION\)=.*,\1=$(PKG_VERSION),' $(TOOLCHAIN_DIR)/info.mk
	$(SED) 's,^\(LIBC_PATCHVER\)=.*,\1=$(PKG_EXTRAVERSION),' $(TOOLCHAIN_DIR)/info.mk
endef

define Build/Prepare/Snapshot
	$(PKG_UNPACK)
	rm -Rf $(PKG_BUILD_DIR)
	ln -snf $(BUILD_DIR_TOOLCHAIN)/uClibc $(PKG_BUILD_DIR)
	$(Build/Patch)
	$(if $(QUILT),touch $(PKG_BUILD_DIR)/.quilt_used)
endef

define Build/Prepare/V_0_9_28
	$(SED) 's,^KERNEL_SOURCE=.*,KERNEL_SOURCE=\"$(BUILD_DIR_TOOLCHAIN)/linux\",g' \
		$(PKG_BUILD_DIR)/.config
 ifeq ($(CONFIG_SOFT_FLOAT),y)
	$(SED) 's,.*HAS_FPU.*,HAS_FPU=n\nUCLIBC_HAS_FLOATS=y\nUCLIBC_HAS_SOFT_FLOAT=y,g' $(PKG_BUILD_DIR)/.config
 endif
 ifeq ($(CONFIG_C99_MATH),y)
	$(SED) 's,.*DO_C99_MATH.*,DO_C99_MATH=y,g' $(PKG_BUILD_DIR)/.config
 endif
endef

define Build/Prepare
	$(call Build/SetToolchainInfo)
	@echo prepare uClibc version $(PKG_VERSION)$(PKG_EXTRAVERSION)  
 ifeq ($(PKG_VERSION_SNAPSHOT),y)
	$(call Build/Prepare/Snapshot)
 else
	$(call Build/Prepare/Default)
	ln -snf $(PKG_NAME)-$(PKG_VERSION) $(BUILD_DIR_TOOLCHAIN)/$(PKG_NAME)
 endif
	$(CP) ./$(CONFIG_DIR)/$(ARCH)$(if $(wildcard $(CONFIG_DIR)/$(ARCH).$(BOARD)),.$(BOARD)) $(PKG_BUILD_DIR)/.config
 ifeq ($(PKG_VERSION),0.9.28)
	$(call Build/Prepare/V_0_9_28)
 endif
	$(SED) 's,^KERNEL_HEADERS=.*,KERNEL_HEADERS=\"$(BUILD_DIR_TOOLCHAIN)/linux-dev/usr/include\",g' \
		-e 's,^.*UCLIBC_HAS_FPU.*,UCLIBC_HAS_FPU=$(if $(CONFIG_SOFT_FLOAT),n,y),g' \
		-e 's,^.*UCLIBC_HAS_SOFT_FLOAT.*,UCLIBC_HAS_SOFT_FLOAT=$(if $(CONFIG_SOFT_FLOAT),y,n),g' \
		-e 's,^.*UCLIBC_HAS_LFS.*,UCLIBC_HAS_LFS=$(if $(CONFIG_LARGEFILE),y,n),g' \
		-e 's,^.*CONFIG_ARM_EABI.*,CONFIG_ARM_EABI=$(if $(CONFIG_EABI_SUPPORT),y,n),g' \
		-e 's,^.*CONFIG_ARM_OABI.*,CONFIG_ARM_OABI=$(if $(CONFIG_EABI_SUPPORT),n,y),g' \
		-e 's,^.*UCLIBC_HAS_SHADOW.*,UCLIBC_HAS_SHADOW=$(if $(CONFIG_SHADOW_PASSWORDS),y,n),g' \
		$(PKG_BUILD_DIR)/.config
	$(MAKE) -C $(PKG_BUILD_DIR)/extra/config conf KBUILD_HAVE_NLS= HOSTCFLAGS="-DKBUILD_NO_NLS"
	PATH=$(TARGET_PATH) $(MAKE) -C $(PKG_BUILD_DIR) \
		PREFIX="$(BUILD_DIR_TOOLCHAIN)/$(LIBC)-dev/" \
		DEVEL_PREFIX=/usr/ \
		RUNTIME_PREFIX="$(BUILD_DIR_TOOLCHAIN)/$(LIBC)-dev/" \
		HOSTCC="$(HOSTCC)" \
		CC="$(TARGET_CC)" \
		CPU_CFLAGS="$(TARGET_CFLAGS)" \
		pregen \
		$(if $(CONFIG_UCLIBC_VERSION_0_9_28),install_dev,install_headers)
endef

define Build/Configure
endef

UCLIBC_MAKE := PATH=$(TARGET_PATH) $(MAKE) -C $(PKG_BUILD_DIR) \
	$(TARGET_CONFIGURE_OPTS) \
	DEVEL_PREFIX=/usr/ \
	RUNTIME_PREFIX=/ \
	HOSTCC="$(HOSTCC)" \
	CPU_CFLAGS="$(TARGET_CFLAGS)" \
	DOSTRIP=""

define Build/Compile
	$(SED) 's,^CROSS=.*,CROSS=$(TARGET_CROSS),g' $(PKG_BUILD_DIR)/Rules.mak
	$(UCLIBC_MAKE) PREFIX= all
	$(UCLIBC_MAKE) PREFIX="$(TOOLCHAIN_DIR)/" install_runtime install_dev
	ln -sf ../../lib/libc.so.0 $(TOOLCHAIN_DIR)/usr/lib/libc.so
#	( cd $(TOOLCHAIN_DIR) ; \
#		for d in lib usr/lib ; do \
#		  for f in libc.so libpthread.so libgcc_s.so ; do \
#		    if [ -f $$$$d/$$$$f -a ! -L $$$$d/$$$$f ] ; then \
#		      $(SED) 's,/usr/lib/,,g;s,/lib/,,g' $$$$d/$$$$f ; \
#		    fi \
#		  done \
#		done \
#	)
endef

define Build/Install
	$(UCLIBC_MAKE) PREFIX= utils
	$(INSTALL_DIR) $(TOOLCHAIN_DIR)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/utils/ldd \
		$(TOOLCHAIN_DIR)/usr/bin/
	$(INSTALL_DIR) $(TOOLCHAIN_DIR)/sbin
	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/utils/ldconfig \
		$(TOOLCHAIN_DIR)/sbin/
endef

define Build/Clean
	rm -rf \
		$(PKG_BUILD_DIR) \
		$(BUILD_DIR_TOOLCHAIN)/$(PKG_NAME) \
		$(BUILD_DIR_TOOLCHAIN)/$(LIBC)-dev
endef

$(eval $(call HostBuild))
