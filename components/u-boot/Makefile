ifndef PETALINUX
$(error "Error: PETALINUX environment variable not set.  Change to the root of your PetaLinux install, and source the settings.sh file")
endif

TYPE:=u-boot

dir_u_boot_y:=

include Makefile.autoinclude

ifndef PROOT
$(error "Failed to build $(TYPE). No PetaLinux project is specified")
endif

ifndef INSTANCE
$(error "Failed to build $(TYPE). No instance is specified")
endif

ifndef CONFIGROOT
$(error "Failed to build $(INSTANCE). No configuration root directory is specified")
endif

ifndef IMAGEROOT
$(error "Failed to build $(INSTANCE). No image directory is specified")
endif

ifndef SUBSYSTEM_CONFIG
$(error "Failed to build $(INSTANCE). No subsystem configuration file is specified")
endif

ifndef HW_DESCRIPTION
$(error "Failed to build $(INSTANCE). No hw-description is sepecified")
endif

include $(PETALINUX)/etc/build/common.mk

ifndef ARCH
$(error "Failed to build $(INSTANCE). No ARCH specified!")
endif

ifndef ENDIAN
$(error "Failed to build $(INSTANCE). No endian specified")
endif

dir_y:=$(dir_u_boot_y)
ifeq ($(dir_y),)
$(error "Failed to build U-Boot. No selected U-Boot")
endif


UBOOTSRC:=$(firstword $(dir_y))
BUILDROOT:=$(CURDIR)
UBOOTOUT:=$(BUILDROOT)/$(shell basename $(UBOOTSRC))

include $(PETALINUX)/etc/build/common.mk

UBOOT_VENDOR=xilinx
ifeq ($(ARCH),microblaze)
UBOOT_PLATFORM=microblaze-generic
UBOOT_BOARD=microblaze-generic
else
ifeq ($(ARCH),arm)
UBOOT_PLATFORM=zynq_zc702
UBOOT_BOARD=zynq
else
ifeq ($(ARCH),aarch64)
UBOOT_PLATFORM=xilinx_zynqmp_ep
UBOOT_BOARD=zynqmp
else
$(error "Failed to build $(INSTANCE), unknown ARCH: $(ARCH)")
endif
endif
endif

UBOOT_CONFIG_TARGET=$(shell grep "^CONFIG_SUBSYSTEM_UBOOT_CONFIG_TARGET=" $(SUBSYSTEM_CONFIG) | cut -d "=" -f 2)
UBOOT_AUTO_CONFIG=$(shell grep "^CONFIG_SUBSYSTEM_AUTOCONFIG_U__BOOT=y" "$(SUBSYSTEM_CONFIG)" | cut -d "=" -f 2)

ifneq ($(UBOOT_CONFIG_TARGET),)
UBOOT_PLATFORM:=$(subst ",,$(UBOOT_CONFIG_TARGET))
#else
#ifeq ($(ARCH),aarch64)
#$(error "PetaLinux auto u-boot config doesn't support for AARCH64, please run petalinux-config to set a uboot config target.")
#endif
endif

UBOOTBUILDSRC=$(BUILDROOT)/src/$(shell basename $(UBOOTSRC))
UBOOT_BOARD_DIR=$(UBOOTBUILDSRC)/board/$(UBOOT_VENDOR)/$(UBOOT_BOARD)
UBOOT_GENERIC_CONFIG_H=platform_top.h

UBOOT_HSM_TCL_SCRIPT=$(PETALINUX)/etc/hsm/scripts/petalinux_hsm_bridge.tcl

MAKEUBOOT=$(MAKE) O=$(UBOOTOUT) -C $(UBOOTBUILDSRC) -j$(HOST_NCPU)

.PHONY: all build pre-build post-build config pre-config oldconfig post-config clean distclean mrproper

all: build post-build

oldconfig $(UBOOT_CONFIG): $(UBOOTSRC) $(UBOOTOUT) $(UBOOTOUT)/.config autogen-config
	@$(call do_info, "$@ $(INSTANCE)")
	yes "" | env KCONFIG_CONFIG="$(UBOOT_CONFIG)" make -C $(UBOOTSRC) oldconfig O=$(UBOOTOUT)

config: $(UBOOTSRC) $(UBOOTOUT) $(UBOOTOUT)/.config
	@$(call do_info, "$@ $(INSTANCE)")
	env KCONFIG_CONFIG=$(UBOOT_CONFIG) make -C $(UBOOTSRC) menuconfig O=$(UBOOTOUT)

pre-config:

post-config:

%_defconfig: $(UBOOTSRC) $(UBOOTOUT) $(UBOOTOUT)/.config
	@$(call do_info,"$@ $(INSTANCE)")
	$(MAKE) O=$(UBOOTOUT) -C $(UBOOTSRC) -j$(HOST_NCPU) $@ || exit 255
	cp "$(UBOOTOUT)/.config" "$(UBOOT_CONFIG)"

defconfig : $(UBOOTSRC) $(UBOOTOUT)
	@$(call do_info,"$@ $(INSTANCE)")
	$(MAKE) O=$(UBOOTOUT) -C $(UBOOTSRC) -j$(HOST_NCPU) $(UBOOT_PLATFORM)_config || exit 255
	cp "$(UBOOTOUT)/.config" "$(UBOOT_CONFIG)"

.PHONY: autogen-config
autogen-config: $(SYSTEM_HDF) $(SUBSYSTEM_CONFIG) $(UBOOTOUT)/.config
ifeq ($(UBOOT_AUTO_CONFIG),y)
ifeq ($(UBOOT_CONFIG_TARGET),)
	@$(call do_info,"generate $(INSTANCE) board header files")
	hsi -mode batch -source $(UBOOT_HSM_TCL_SCRIPT) -notrace -nojournal -nolog -tclargs "-c $(SUBSYSTEM_CONFIG) -a u-boot_bsp -hdf $(SYSTEM_HDF) -o $(UBOOT_CONFIGDIR) -d $(PETALINUX)/etc/hsm/data"
	# remove tail spaces
	sed -i 's: \+$$::g' $(UBOOT_CONFIGDIR)/platform-auto.h
	if grep -q -e "^CONFIG_SPL=y" $(UBOOT_CONFIG); then \
		sed -i -e "s/\(CONFIG_SPL\)=.*/\# \1 is not set/g" $(UBOOT_CONFIG); \
	fi
endif
endif

pre-build:

build:$(UBOOTOUT)/u-boot

post-build:

$(UBOOTOUT)/.config: $(UBOOTSRC) $(UBOOTOUT)
	@$(call do_info,"generate $(INSTANCE) configuration files")
	$(MAKE) O=$(UBOOTOUT) -C $(UBOOTSRC) -j$(HOST_NCPU) $(UBOOT_PLATFORM)_config || exit 255
	if [ -f "$(UBOOT_CONFIG)" ]; then \
		cp "$(UBOOT_CONFIG)" $@; \
	else \
		if [ ! -d "$$(dirname $(UBOOT_CONFIG))" ]; then \
			mkdir -p "$$(dirname $(UBOOT_CONFIG))"; \
		fi; \
		cp $@ "$(UBOOT_CONFIG)"; \
	fi

clean:
	@$(call do_info, "$@ $(INSTANCE)")
	$(MAKEUBOOT) $@

mrproper distclean:

$(UBOOTOUT)/u-boot: $(UBOOTOUT) $(UBOOTBUILDSRC) $(UBOOTOUT)/.config
	@$(call do_info,"build $(INSTANCE)")
	$(MAKEUBOOT) all

.PHONY: $(UBOOTBUILDSRC)
$(UBOOTBUILDSRC): $(UBOOTSRC) $(UBOOTOUT)/.config $(DTB)
	@$(call do_info, "update $(INSTANCE) source")
	if [ ! -d "$(UBOOTBUILDSRC)" ]; then \
		mkdir -p "$(UBOOTBUILDSRC)"; \
	fi; \
	if [ -f "$(UBOOTSRC)/.gitignore" ]; then \
		grep -v -e "^#" "$(UBOOTSRC)/.gitignore" > u_boot_source_exclude; \
	else \
		echo "" > u_boot_source_exclude; \
	fi; \
	rsync -a --exclude-from=u_boot_source_exclude --filter=':C .git/ RCS SCCS CVS CVS.adm RCSLOG cvslog.* *.o' "$(UBOOTSRC)"/ "$(UBOOTBUILDSRC)"/
	chmod -R a+w "$(UBOOTBUILDSRC)"/include/configs "$(UBOOT_BOARD_DIR)"
	if [ -f "$(UBOOT_CONFIGDIR)/config.mk" ]; then \
		rsync "$(UBOOT_CONFIGDIR)/config.mk"  "$(UBOOT_BOARD_DIR)/config.mk"; \
	fi
	uboot_arch=`grep -e "^CONFIG_SYS_ARCH=" "$(UBOOTOUT)/.config" | sed -e "s/.*=\\"\\(.*\\)\\"/\\1/g"`; \
	uboot_dts=`grep -e "^CONFIG_DEFAULT_DEVICE_TREE=" "$(UBOOTOUT)/.config" | sed -e "s/.*=\\"\\(.*\\)\\"/\\1/g"`; \
	dtc -I dtb -O dts -o "$(UBOOTBUILDSRC)/arch/$${uboot_arch}/dts/$${uboot_dts}.dts" "$(DTB)"
ifeq ($(UBOOT_CONFIG_TARGET),)
	if [ -f "$(UBOOT_CONFIGDIR)/platform-top.h" ]; then \
		rsync "$(UBOOT_CONFIGDIR)"/*.h "$(UBOOTBUILDSRC)/include/configs"/; \
		uboot_sysconf_plat=`grep -e "^CONFIG_SYS_CONFIG_NAME=" "$(UBOOTOUT)/.config" | sed -e "s/.*=\\"\\(.*\\)\\"/\\1/g"`; \
		if [ ! -f "$(UBOOTBUILDSRC)/include/configs/$${uboot_sysconf_plat}.h" ]; then \
			echo "ERROR: failed to get the u-boot sys config."; \
			exit 255; \
		fi; \
		mv "$(UBOOTBUILDSRC)/include/configs/platform-top.h" "$(UBOOTBUILDSRC)/include/configs/$${uboot_sysconf_plat}.h"; \
	fi
endif

pre-install:
install:$(IMAGEROOT) $(UBOOTOUT)/u-boot
	@$(call do_info,"$@ $(INSTANCE)")
	cp $(UBOOTOUT)/u-boot.srec $(IMAGEROOT); \
	cp $(UBOOTOUT)/u-boot.bin $(IMAGEROOT); \
	if [ -f $(UBOOTOUT)/u-boot.elf ]; then \
		cp $(UBOOTOUT)/u-boot.elf $(IMAGEROOT)/u-boot.elf; \
	else \
		cp $(UBOOTOUT)/u-boot $(IMAGEROOT)/u-boot.elf; \
	fi

post-install:

.PHONY: %_only
%_only: $(UBOOTBUILDSRC)
	$(eval TARGET:=$(subst _only,,$@))
	@$(call do_info,"$(TARGET) $(INSTANCE)")
	$(MAKEUBOOT) $(TARGET)

$(IMAGEROOT):
	mkdir -p "$@"

$(UBOOTOUT):
	mkdir -p "$@"

help:
	@echo ""
	@echo "Quick reference for various supported build targets for u-boot."
	@echo "----------------------------------------------------"
	@echo "  clean               clean out compiled files, but not config"
	@echo "  distclean           clean out all build files"
	@echo "  build               build u-boot"
	@echo "  install               install u-boot images to images/ directory"
	@echo "  <UBOOT_TARGET>_only make a specific u-boot target"
	@echo "  all                 configure build environment and build"
