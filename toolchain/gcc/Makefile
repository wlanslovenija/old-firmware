#
# Copyright (C) 2002-2003 Erik Andersen <andersen@uclibc.org>
# Copyright (C) 2004 Manuel Novoa III <mjn3@uclibc.org>
# Copyright (C) 2005-2006 Felix Fietkau <nbd@openwrt.org>
# Copyright (C) 2006-2009 OpenWrt.org
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

include $(TOPDIR)/rules.mk

PKG_NAME:=gcc
PKG_VERSION:=$(call qstrip,$(CONFIG_GCC_VERSION))

PKG_SOURCE_URL:=@GNU/gcc/gcc-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2

ifeq ($(PKG_VERSION),3.4.6)
  PKG_MD5SUM:=4a21ac777d4b5617283ce488b808da7b
endif
ifeq ($(PKG_VERSION),4.1.2)
  PKG_MD5SUM:=a4a3eb15c96030906d8494959eeda23c
endif
ifeq ($(PKG_VERSION),4.2.4)
  PKG_MD5SUM:=d79f553e7916ea21c556329eacfeaa16
endif
ifeq ($(PKG_VERSION),4.3.1)
  PKG_MD5SUM:=4afa0290cc3a41ac8822666f1110de98
endif
ifeq ($(PKG_VERSION),4.3.2)
  PKG_MD5SUM:=5dfac5da961ecd5f227c3175859a486d
endif

PATCH_DIR=./patches/$(PKG_VERSION)

STAGING_DIR_HOST:=$(TOOLCHAIN_DIR)
BUILD_DIR_HOST:=$(BUILD_DIR_TOOLCHAIN)

override CONFIG_AUTOREBUILD=

include $(INCLUDE_DIR)/host-build.mk

STAMP_BUILT:=$(TOOLCHAIN_DIR)/stamp/.gcc-initial_installed

PKG_BUILD_DIR0:=$(PKG_BUILD_DIR)-minimal
PKG_BUILD_DIR1:=$(PKG_BUILD_DIR)-initial
PKG_BUILD_DIR2:=$(PKG_BUILD_DIR)-final

SEP:=,
TARGET_LANGUAGES:="c$(if $(CONFIG_INSTALL_LIBSTDCPP),$(SEP)c++)$(if $(CONFIG_INSTALL_LIBGCJ),$(SEP)java)"

GCC_CONFIGURE_COMMON:= \
	SHELL="$(BASH)" \
	$(PKG_BUILD_DIR)/configure \
		--prefix=$(TOOLCHAIN_DIR)/usr \
		--build=$(GNU_HOST_NAME) \
		--host=$(GNU_HOST_NAME) \
		--target=$(REAL_GNU_TARGET_NAME) \
		--with-gnu-ld \
		--enable-target-optspace \
		--disable-libgomp \
		--disable-libmudflap \
		--disable-multilib \
		--disable-nls \
		$(SOFT_FLOAT_CONFIG_OPTION) \
		$(call qstrip,$(CONFIG_EXTRA_GCC_CONFIG_OPTIONS)) \

ifneq ($(CONFIG_GCC_VERSION_4_3),)
	GCC_BUILD_TARGET_LIBGCC:=y
	GCC_CONFIGURE_COMMON+= \
		--with-gmp=$(TOPDIR)/staging_dir/host \
		--with-mpfr=$(TOPDIR)/staging_dir/host \
		--disable-decimal-float
endif

ifneq ($(CONFIG_SSP_SUPPORT),)
	GCC_CONFIGURE_COMMON+= \
		--enable-libssp
else
	GCC_CONFIGURE_COMMON+= \
		--disable-libssp
endif

ifneq ($(CONFIG_EXTRA_TARGET_ARCH),)
	GCC_CONFIGURE_COMMON+= \
		--enable-biarch \
		--enable-targets=$(call qstrip,$(CONFIG_EXTRA_TARGET_ARCH_NAME))-linux-$(TARGET_SUFFIX)
endif

ifeq ($(LIBC),uClibc)
	GCC_CONFIGURE_COMMON+= \
		--disable-__cxa_atexit
else
	GCC_CONFIGURE_COMMON+= \
		--enable-__cxa_atexit
endif

GCC_CONFIGURE_STAGE0:= \
	$(GCC_CONFIGURE_COMMON) \
		--with-newlib \
		--without-headers \
		--enable-languages=c \
		--disable-libssp \
		--disable-shared \
		--disable-threads \

GCC_CONFIGURE_STAGE1:= \
	$(GCC_CONFIGURE_COMMON) \
		--with-newlib \
		--with-sysroot=$(BUILD_DIR_TOOLCHAIN)/$(LIBC)-dev \
		--enable-languages=c \
		--disable-shared \
		--disable-threads \

GCC_CONFIGURE_STAGE2:= \
	$(GCC_CONFIGURE_COMMON) \
		--enable-languages=$(TARGET_LANGUAGES) \
		--enable-shared \
		--enable-threads \
		--with-slibdir=$(TOOLCHAIN_DIR)/lib \

GCC_MAKE:= \
	export SHELL="$(BASH)"; \
	$(MAKE) \


define Build/SetToolchainInfo
	$(SED) 's,TARGET_CROSS=.*,TARGET_CROSS=$(REAL_GNU_TARGET_NAME)-,' $(TOOLCHAIN_DIR)/info.mk
	$(SED) 's,GCC_VERSION=.*,GCC_VERSION=$(PKG_VERSION),' $(TOOLCHAIN_DIR)/info.mk
endef


define Stage0/Configure
	mkdir -p $(PKG_BUILD_DIR0)
	(cd $(PKG_BUILD_DIR0); rm -f config.cache; \
		$(GCC_CONFIGURE_STAGE0) \
	);
endef

define Stage0/Compile
	$(GCC_MAKE) -C $(PKG_BUILD_DIR0) all-gcc
endef

define Stage0/Install
	$(GCC_MAKE) -C $(PKG_BUILD_DIR0) install-gcc
endef


define Stage1/Configure
	mkdir -p $(PKG_BUILD_DIR1)
	(cd $(PKG_BUILD_DIR1); rm -f config.cache; \
		$(GCC_CONFIGURE_STAGE1) \
	);
endef

define Stage1/Compile
	$(CP) $(BUILD_DIR_TOOLCHAIN)/linux-dev/* $(BUILD_DIR_TOOLCHAIN)/$(LIBC)-dev/
	$(GCC_MAKE) -C $(PKG_BUILD_DIR1) \
		all-build-libiberty \
		all-gcc \
		$(if $(GCC_BUILD_TARGET_LIBGCC),all-target-libgcc)
endef

define Stage1/Install
	$(GCC_MAKE) -C $(PKG_BUILD_DIR1) \
		install-gcc \
		$(if $(GCC_BUILD_TARGET_LIBGCC),install-target-libgcc)
	
	# XXX: glibc insists on linking against libgcc_eh
	( cd $(TOOLCHAIN_DIR)/usr/lib/gcc/$(REAL_GNU_TARGET_NAME)/$(PKG_VERSION) ; \
		[ -e libgcc_eh.a ] || ln -sf libgcc.a libgcc_eh.a ; \
	)
endef


define Stage2/Configure
	mkdir -p $(PKG_BUILD_DIR2) $(TOOLCHAIN_DIR)/usr/$(REAL_GNU_TARGET_NAME)
	# Important!  Required for limits.h to be fixed.
	rm -rf $(TOOLCHAIN_DIR)/usr/$(REAL_GNU_TARGET_NAME)/sys-include
	ln -sf ../include $(TOOLCHAIN_DIR)/usr/$(REAL_GNU_TARGET_NAME)/sys-include
	rm -rf $(TOOLCHAIN_DIR)/usr/$(REAL_GNU_TARGET_NAME)/lib
	ln -sf ../lib $(TOOLCHAIN_DIR)/usr/$(REAL_GNU_TARGET_NAME)/lib
	(cd $(PKG_BUILD_DIR2); rm -f config.cache; \
		$(GCC_CONFIGURE_STAGE2) \
	);
endef

define Stage2/Compile
	$(GCC_MAKE) -C $(PKG_BUILD_DIR2) all
endef

define SetupExtraArch
	for app in $(TOOLCHAIN_DIR)/usr/bin/$(OPTIMIZE_FOR_CPU)*-{gcc,gcc-*,g++}; do \
		[ -e $$$$app ] || continue; \
		old_base=$$$$(basename $$$$app); \
		new_base=$(call qstrip,$(CONFIG_EXTRA_TARGET_ARCH_NAME))-$$$${old_base##$(OPTIMIZE_FOR_CPU)-}; \
		sed -e "s/@CC_BASE@/$$$$old_base/" \
			-e 's/@EXTRA_ARCH_OPTS@/$(call qstrip,$(CONFIG_EXTRA_TARGET_ARCH_OPTS))/' \
			 ./files/alternate-arch-cc.in > \
			 $(TOOLCHAIN_DIR)/usr/bin/$$$$new_base; \
		chmod a+x $(TOOLCHAIN_DIR)/usr/bin/$$$$new_base; \
	done
endef

define Stage2/Install
	$(GCC_MAKE) -C $(PKG_BUILD_DIR2) install
	# Set up the symlinks to enable lying about target name.
	set -e; \
	(cd $(TOOLCHAIN_DIR)/usr; \
		ln -sf $(REAL_GNU_TARGET_NAME) $(GNU_TARGET_NAME); \
		cd bin; \
		for app in $(REAL_GNU_TARGET_NAME)-* ; do \
			ln -sf $$$${app} \
		   	$(GNU_TARGET_NAME)$$$${app##$(REAL_GNU_TARGET_NAME)}; \
		done; \
	);
	$(if $(CONFIG_EXTRA_TARGET_ARCH),$(call SetupExtraArch))
endef


define Build/Prepare
	$(call Build/SetToolchainInfo)
	$(call Build/Prepare/Default)
	ln -snf $(PKG_NAME)-$(PKG_VERSION) $(BUILD_DIR_TOOLCHAIN)/$(PKG_NAME)
	$(CP) $(SCRIPT_DIR)/config.{guess,sub} $(PKG_BUILD_DIR)/
	$(SED) 's,\(version_string.. = "[0-9\.]*\).*\(";\),\1 (OpenWrt-2.0)\2,' $(PKG_BUILD_DIR)/gcc/version.c
	$(SED) 's,\(bug_report_url.. = "\).*\(";\),\1<URL:https://dev.openwrt.org/>\2,' $(PKG_BUILD_DIR)/gcc/version.c
	(cd $(PKG_BUILD_DIR)/libstdc++-v3; autoconf;);
	$(SED) 's,gcc_no_link=yes,gcc_no_link=no,' $(PKG_BUILD_DIR)/libstdc++-v3/configure
	$(call Stage0/Configure)
	$(call Stage0/Compile)
	$(call Stage0/Install)
endef

define Build/Configure
endef

define Build/Compile
	$(call Stage1/Configure)
	$(call Stage1/Compile)
	$(call Stage1/Install)
endef

define Build/Install
	$(call Stage2/Configure)
	$(call Stage2/Compile)
	$(call Stage2/Install)
endef

define Build/Clean
	rm -rf \
		$(PKG_BUILD_DIR) \
		$(PKG_BUILD_DIR0) \
		$(PKG_BUILD_DIR1) \
		$(PKG_BUILD_DIR2) \
		$(BUILD_DIR_TOOLCHAIN)/$(PKG_NAME) \
		$(TOOLCHAIN_DIR)/usr/$(REAL_GNU_TARGET_NAME) \
		$(TOOLCHAIN_DIR)/usr/bin/$(REAL_GNU_TARGET_NAME)-gc* \
		$(TOOLCHAIN_DIR)/usr/bin/$(REAL_GNU_TARGET_NAME)-c*
endef

$(eval $(call HostBuild))
