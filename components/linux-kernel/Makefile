
ifndef PETALINUX
$(error "Error: PETALINUX environment variable not set.  Change to the root of your PetaLinux install, and source the settings.sh file")
endif

override TYPE=linux-kernel

dir_linux_kernel_y:=

include Makefile.autoinclude

ifndef PROOT
$(error "Failed to build $(TYPE). No PetaLinux project is specified")
endif

ifndef INSTANCE
$(error "Failed to build $(TYPE). No instance specified")
endif

ifndef IMAGEROOT
$(error "Failed to build $(INSTANCE). No image directory is specified")
endif

ifndef SUBSYSTEM_CONFIG
$(error "Failed to build $(INSTANCE). No subsystem configuration file is specified")
endif

ifndef CONFIGFILE
$(error "Failed to build $(INSTANCE). No configuration file is specified")
endif

dir_y:=$(dir_linux_kernel_y)
ifeq ($(dir_y),)
$(error "Failed to build $(INSTANCE). No selected Linux kernel")
endif

include $(PETALINUX)/etc/build/common.mk

ifndef PLNX_SUBSYSTEM_ARCH
$(error "Failed to build $(INSTANCE). No ARCH specified!")
endif

ifndef ENDIAN
$(error "Failed to build $(INSTANCE). No endian specified")
endif

LINUXSRC=$(firstword $(dir_y))
LINUXOUT=$(CURDIR)/$(shell basename $(LINUXSRC))

ifeq ($(PLNX_SUBSYSTEM_ARCH),aarch64)
ARCH:=arm64
else
ARCH:=$(PLNX_SUBSYSTEM_ARCH)
endif

MAKEARCH:=$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
ifneq ($(SUBARCH),)
MAKEARCH += SUBARCH=$(SUBARCH)
endif

MAKELINUX:= $(MAKEARCH) O=$(LINUXOUT) KCONFIG_CONFIG="$(LINUXOUT)"/.config -j$(HOST_NCPU) -C $(LINUXSRC)

INITRAMFS_SOURCE=$(shell env SUBSYSTEM=$(SUBSYSTEM) PROOT=$(PROOT) SUBSYSTEM_CONFIG=$(SUBSYSTEM_CONFIG) LINUX_CONFIG=$(CONFIGFILE) bash ./get_initramfs_file)

.PHONY: all build pre-build clean distclean mrproper pre-config post-config modules modules_install defconfig %_defconfig

all: build 

pre-build:

build: $(LINUXOUT)/vmlinux linux_exportdir

post-build:

oldconfig: $(LINUXOUT)
	@$(call do_info, "$@ $(INSTANCE)")
	yes "" | env KCONFIG_CONFIG="$(LINUX_CONFIG)" make -C $(LINUXSRC) oldconfig O=$(LINUXOUT)

config: $(LINUXOUT)
	@$(call do_info, "$@ $(INSTANCE)")
	env KCONFIG_CONFIG=$(LINUX_CONFIG) make -C $(LINUXSRC) menuconfig O=$(LINUXOUT)

$(LINUXOUT)/.config: $(LINUXOUT) $(LINUX_CONFIG)
	rsync $(CONFIGFILE) $(LINUXOUT)/.config

pre-config:

post-config:

%_defconfig: $(LINUXOUT)
	@$(call do_info,"$@ $(INSTANCE)")
	$(MAKELINUX) $@ || exit 255
	cp "$(LINUXOUT)/.config" "$(LINUX_CONFIG)"

defconfig: $(LINUXOUT)
	@$(call do_info,"$@ $(INSTANCE)")
	defconfig_data=$$(petalinux-simple-yaml-parser defconfig.yaml); \
	sysarch=$(PLNX_SUBSYSTEM_ARCH); \
	default_defconfig=$$(echo "$${defconfig_data}" | egrep "^subsystem_arch__$${sysarch}__default_defconfig=" | head -n1 | awk -F"=" '{print $$2}' | tr -d '"'); \
	if [ -z "$${default_defconfig}" ]; then \
		echo "ERROR: No default defconfig is found for $${sysarch}."; \
		exit 255; \
	fi; \
	$(call do_info,"$${default_defconfig} $(INSTANCE)"); \
	$(MAKELINUX) $${default_defconfig} || exit 255
	cp "$(LINUXOUT)/.config" "$(LINUX_CONFIG)"

mergeconfig: $(LINUXOUT)
	@$(call do_info, "$@ $(INSTANCE)")
	configfiles=""; \
	while read -r line; do \
		if [ -z "$${line}" ]; then \
			continue; \
		fi; \
		configfiles="$${configfiles} \"$${line}\""; \
	done < mergeconfig_files; \
	if [ -z "$${configfiles}" ]; then \
		echo "ERROR:No config files has been specified for mergeconfig."; \
		exit 255; \
	fi; \
	eval ARCH=$(ARCH) $(LINUXSRC)/scripts/kconfig/merge_config.sh -m -O "$(LINUXOUT)" "$${configfiles}" || exit 255
	cp $(LINUXOUT)/.config "$(LINUX_CONFIG)"
	yes "" | env KCONFIG_CONFIG="$(LINUX_CONFIG)" make -C $(LINUXSRC) oldconfig O=$(LINUXOUT)

clean:
	@$(call do_info,"clean $(INSTANCE)")
	$(MAKELINUX) clean

distclean mrproper:

%_only: $(LINUXOUT)/.config $(CURDIR)/boot $(CURDIR)/usr linux_exportdir
	$(eval TARGET:=$(subst _only,,$@))
	@$(call do_info,"$(TARGET) $(INSTANCE)")
	$(MAKELINUX) $(TARGET) 

$(LINUXOUT)/vmlinux: $(LINUXOUT) $(LINUXOUT)/.config $(INITRAMFS_SOURCE) linux_exportdir
	@$(call do_info,"build $(INSTANCE)")
	$(MAKELINUX) all

ifneq ($(INITRAMFS_SOURCE),)
$(INITRAMFS_SOURCE):
	initramfs_source_dir=$$(dirname "$(INITRAMFS_SOURCE)"); \
	if [ ! -d "$${initramfs_source_dir}" ]; then \
		mkdir -p "$${initramfs_source_dir}"; \
	fi; \
	touch "$@"
endif

pre-install:
install: $(IMAGEROOT) $(LINUXOUT)/System.map
	@$(call do_info, "$@ $(INSTANCE)")
	if [ ! -f $(LINUXOUT)/vmlinux ]; then \
		echo "ERROR: $(LINUXOUT)/vmlinux is not found. Please build linux first!"; \
		exit 255; \
	fi
	cp "$(LINUXOUT)"/vmlinux "$(IMAGEROOT)"/
	cp "$(LINUXOUT)"/System.map "$(IMAGEROOT)"/System.map.linux

post-install:

$(LINUXOUT) $(LINUXOUT)/modules:
	mkdir -p "$@"
	mkdir -p $(LINUXOUT)/modules
	rm -rf link-to-kernel-build
	ln -s $(LINUXOUT) link-to-kernel-build

linux_exportdir:
	rm -rf $(CURDIR)/boot $(CURDIR)/usr
	ln -s "$(LINUXOUT)/arch/$(ARCH)/boot" "$(CURDIR)/boot"
	ln -s "$(LINUXOUT)/usr" "$(CURDIR)/usr"

$(IMAGEROOT):
	mkdir -p "$@"

help:
	@echo ""
	@echo "Quick reference for various supported build targets for $(TYPE)."
	@echo "----------------------------------------------------"
	@echo "  clean              clean out compiled files, but not config"
	@echo "  distclean          clean out all build files"
	@echo "  config             configure linux build enviroment"
	@echo "  build              build Linux"
	@echo "  post_build         install Linux images to images/ directory"
	@echo "  <LINUX_TARGET>_only  make a specific Linux kernel target"
	@echo "  all                configure build environment and build"
