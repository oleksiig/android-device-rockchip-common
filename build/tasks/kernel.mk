#
# Copyright (C) 2021 The Android Open-Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

ifneq (,$(filter $(TARGET_PRODUCT),rockpro64 x3399headunit x3399evk))
ifneq ($(strip $(TARGET_NO_KERNEL)),true)

ifeq ($(BOARD_KERNEL_TOOLCHAIN),)
$(error "BOARD_KERNEL_TOOLCHAIN is not set for target product $(TARGET_PRODUCT)")
endif

ifeq ($(BOARD_KERNEL_SOURCE),)
$(error "BOARD_KERNEL_SOURCE is not set for target product $(TARGET_PRODUCT)")
endif

ifeq ($(BOARD_KERNEL_CONFIG),)
$(error "BOARD_KERNEL_CONFIG is not set for target product $(TARGET_PRODUCT)")
endif

ifeq ($(BOARD_KERNEL_OBJ_OUT),)
$(error "BOARD_KERNEL_OBJ_OUT is not set for target product $(TARGET_PRODUCT)")
endif

# ----------------------------------------------------------------------
KERNEL_MAKE_JOBS            := `/usr/bin/nproc --ignore=3`
KERNEL_CLANG                := /usr/bin/clang-10
KERNEL_MAKE                 := /usr/bin/make
KERNEL_BUILD_TOOLS          := LEX=/usr/bin/flex YACC=/usr/bin/bison M4=/usr/bin/m4
KERNEL_BUILD_TOOLS          += PERL=/usr/bin/perl LZ4=/usr/bin/lz4c
KERNEL_INSTALLED_TARGET     := $(PRODUCT_OUT)/kernel
KERNEL_CROSS_COMPILE        := $(abspath $(BOARD_KERNEL_TOOLCHAIN))
KERNEL_OBJ_OUT_ABS          := $(abspath $(BOARD_KERNEL_OBJ_OUT))
KERNEL_CONFIG               := $(BOARD_KERNEL_OBJ_OUT)/.config
KERNEL_IMAGE                := $(BOARD_KERNEL_OBJ_OUT)/arch/$(TARGET_ARCH)/boot/Image.lz4
KERNEL_BUILD_HOST_FLAGS     := HOSTCC=$(KERNEL_CLANG) HOSTCFLAGS="-Wno-unused-command-line-argument"
KERNEL_BUILD_TARGET_FLAGS   := ARCH=$(TARGET_ARCH) CC=$(KERNEL_CLANG) CLANG_TRIPLE=$(KERNEL_CROSS_COMPILE) CROSS_COMPILE=$(KERNEL_CROSS_COMPILE)
KERNEL_BUILD_FLAGS          := $(KERNEL_BUILD_HOST_FLAGS) $(KERNEL_BUILD_TARGET_FLAGS) $(KERNEL_BUILD_TOOLS)

# ----------------------------------------------------------------------
$(BOARD_VENDOR_KERNEL_MODULES): $(KERNEL_INSTALLED_TARGET)
$(BOARD_RECOVERY_KERNEL_MODULES): $(KERNEL_INSTALLED_TARGET)

# ----------------------------------------------------------------------
$(BOARD_KERNEL_OBJ_OUT):
	mkdir -p $(BOARD_KERNEL_OBJ_OUT)

$(KERNEL_CONFIG): $(BOARD_KERNEL_OBJ_OUT)
	$(KERNEL_MAKE) -C $(BOARD_KERNEL_SOURCE) O=$(KERNEL_OBJ_OUT_ABS) $(KERNEL_BUILD_FLAGS) $(BOARD_KERNEL_CONFIG)

$(KERNEL_IMAGE): $(KERNEL_CONFIG)
	$(KERNEL_MAKE) -C $(BOARD_KERNEL_SOURCE) O=$(KERNEL_OBJ_OUT_ABS) $(KERNEL_BUILD_FLAGS) -j $(KERNEL_MAKE_JOBS) Image.lz4 dtbs modules

$(KERNEL_INSTALLED_TARGET): $(KERNEL_IMAGE)
	cp -v $(KERNEL_IMAGE) $(KERNEL_INSTALLED_TARGET)

# ----------------------------------------------------------------------
.PHONY: clean-kernel
clean-kernel:
	rm -rf $(KERNEL_OBJ_OUT) $(KERNEL_INSTALLED_TARGET)

# ----------------------------------------------------------------------
endif # TARGET_NO_KERNEL
endif # TARGET_PRODUCT
