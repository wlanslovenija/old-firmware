#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

IMGNAME:=$(BIN_DIR)/openwrt-$(BOARD)

define imgname
$(IMGNAME)-$(2)-$(patsubst jffs2-%,jffs2,$(1))
endef

VMLINUX:=$(IMGNAME)-vmlinux
ifeq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),y)
	VMLINUX:=$(IMGNAME)-vmlinux-initramfs
endif

define Image/BuildKernel
	cp $(KDIR)/vmlinux.elf $(VMLINUX).elf
	cp $(KDIR)/vmlinux $(VMLINUX).bin
	gzip -9 -c $(KDIR)/vmlinux > $(KDIR)/vmlinux.bin.gz
	$(STAGING_DIR_HOST)/bin/lzma e $(KDIR)/vmlinux $(KDIR)/vmlinux.bin.l7
	$(STAGING_DIR_HOST)/bin/lzma e $(KDIR)/vmlinux -lc1 -lp2 -pb2 $(KDIR)/vmlinux.bin.lzma
	dd if=$(KDIR)/vmlinux.bin.l7 of=$(VMLINUX).lzma bs=65536 conv=sync
	dd if=$(KDIR)/vmlinux.bin.gz of=$(VMLINUX).gz bs=65536 conv=sync
	mkimage -A mips -O linux -T kernel -a 0x80060000 -C gzip -e \
		0x80060000 \
		-n 'MIPS OpenWrt Linux-$(LINUX_VERSION)' \
		-d $(KDIR)/vmlinux.bin.gz $(IMGNAME)-uImage-gzip.bin
	mkimage -A mips -O linux -T kernel -a 0x80060000 -C lzma -e \
		0x80060000 \
		-n 'MIPS OpenWrt Linux-$(LINUX_VERSION)' \
		-d $(KDIR)/vmlinux.bin.lzma $(IMGNAME)-uImage-lzma.bin
endef

define Image/Build/AP81
	cp $(KDIR)/vmlinux $(KDIR)/vmlinux-$(2)
	$(STAGING_DIR_HOST)/bin/patch-cmdline $(KDIR)/vmlinux-$(2) '$(strip $(3))'
	$(STAGING_DIR_HOST)/bin/lzma e $(KDIR)/vmlinux-$(2) $(KDIR)/vmlinux-$(2).bin.lzma
	if [ `stat -c%s "$(KDIR)/vmlinux-$(2).bin.lzma"` -gt 786432 ]; then \
		echo "Warning: $(KDIR)/vmlinux-$(2).bin.lzma is too big"; \
	else if [ `stat -c%s $(KDIR)/root.$(1)` -gt 3080192 ]; then \
		echo "Warning: $(KDIR)/root.$(1) is too big"; \
	else \
		mkimage -A mips -O linux -T kernel -a 0x80060000 -C lzma -e \
			0x80060000 \
			-n 'MIPS OpenWrt Linux-$(LINUX_VERSION)' \
			-d $(KDIR)/vmlinux-$(2).bin.lzma $(KDIR)/vmlinux-$(2).uImage; \
		( \
			dd if=$(KDIR)/vmlinux-$(2).uImage bs=768k conv=sync; \
			dd if=$(KDIR)/root.$(1) bs=3008k conv=sync; \
		) > $(call imgname,$(1),$(2)).bin; \
		cp $(call imgname,$(1),$(2)).bin $(call imgname,$(1),$(2)).webui; \
		echo -n $(4) >> $(call imgname,$(1),$(2)).webui; \
	fi; fi
endef

define Image/Build/MyLoader
	-$(STAGING_DIR_HOST)/bin/mkmylofw -B $(2) \
		-p0x30000:0xc0000:ahl:0x80060000:kernel:$(KDIR)/vmlinux.bin.lzma  \
		-p0xf0000:0:::rootfs:$(KDIR)/root.$(1) \
		$(call imgname,$(1),$(2)).img
endef

define Image/Build/UBNT
	cp $(KDIR)/vmlinux $(KDIR)/vmlinux-$(2)
	$(STAGING_DIR_HOST)/bin/patch-cmdline $(KDIR)/vmlinux-$(2) '$(strip $(3))'
	$(STAGING_DIR_HOST)/bin/lzma e $(KDIR)/vmlinux-$(2) $(KDIR)/vmlinux-$(2).lzma
	dd if=$(KDIR)/vmlinux-$(2).lzma of=$(KDIR)/vmlinux-$(2).bin.lzma bs=64k conv=sync
	-$(STAGING_DIR_HOST)/bin/mkfwimage \
		-B $(4) -v $(5).ar7100.OpenWrt \
		-k $(KDIR)/vmlinux-$(2).bin.lzma \
		-r $(BIN_DIR)/openwrt-$(BOARD)-root.$(1) \
		-o $(BIN_DIR)/openwrt-$(BOARD)-ubnt-$(2)-$(1).bin
endef

define Image/Build/Template/Compex
	$(call Image/Build/MyLoader,$(1),$(2))
endef

define Image/Build/Template/Compex/squashfs
	$(call Image/Build/Template/Compex,squashfs,$(1))
endef

define Image/Build/Template/AP81
	$(call Image/Build/AP81,$(1),$(2),$(3),$(4))
endef

define Image/Build/Template/AP81/squashfs
	$(call Image/Build/Template/AP81,squashfs,$(1),$(2),$(3))
endef

define Image/Build/Template/UBNT
	$(call Image/Build/UBNT,$(1),$(2),$(3),$(4),$(5))
endef

define Image/Build/Template/UBNT/squashfs
	$(call Image/Build/Template/UBNT,squashfs,$(1),$(2),$(3))
endef

define Image/Build/Profile/WP543
	$(call Image/Build/Template/Compex/$(1),wp543)
endef

define Image/Build/Profile/DIR615C1
	$(call Image/Build/Template/AP81/$(1),dir-615c1,board=TEW-632BRP,"AP81-AR9130-RT-070614-02")
endef

define Image/Build/Profile/TEW632BRP
	$(call Image/Build/Template/AP81/$(1),tew-632brp,board=TEW-632BRP,"AP81-AR9130-RT-070614-00")
endef

define Image/Build/Profile/TEW652BRP
	$(call Image/Build/Template/AP81/$(1),tew-652brp,board=TEW-632BRP,"AP81-AR9130-RT-080609-05")
endef

define Image/Build/Profile/UBNTRS
	$(call Image/Build/Template/UBNT,$(1),rs,board=UBNT-RS,RS,RSx)
endef

define Image/Build/squashfs
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
endef

define Image/Build
	$(call Image/Build/$(1))
	dd if=$(KDIR)/root.$(1) of=$(IMGNAME)-root.$(1) bs=128k conv=sync

	$(call Image/Build/Profile/DIR615C1,$(1))
	$(call Image/Build/Profile/TEW632BRP,$(1))
	$(call Image/Build/Profile/TEW652BRP,$(1))
	$(call Image/Build/Profile/UBNTRS,$(1))
	$(call Image/Build/Profile/WP543,$(1))
endef

$(eval $(call BuildImage))
