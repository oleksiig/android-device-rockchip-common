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

# Primary Arch
TARGET_ARCH := arm64
TARGET_ARCH_VARIANT := armv8-a
TARGET_CPU_ABI := arm64-v8a
TARGET_CPU_VARIANT := cortex-a53

# Secondary Arch
TARGET_2ND_ARCH := arm
TARGET_2ND_ARCH_VARIANT := armv8-a
TARGET_2ND_CPU_ABI := armeabi-v7a
TARGET_2ND_CPU_ABI2 := armeabi
TARGET_2ND_CPU_VARIANT := cortex-a53

TARGET_BOARD_PLATFORM := rk3399
TARGET_BOARD_PLATFORM_GPU := mali-t860

USE_OPENGL_RENDERER := true

TARGET_NO_DTIMAGE := true
TARGET_NO_BOOTLOADER := true
TARGET_NO_KERNEL := false
TARGET_NO_RECOVERY := true

TARGET_USES_64_BIT_BINDER := true

TARGET_ENABLE_MEDIADRM_64 := true
TARGET_USES_HWC2 := true
TARGET_USE_PAN_DISPLAY := true

BOARD_USES_GENERIC_AUDIO := true
BOARD_USES_DRM_HWCOMPOSER := true
BOARD_USE_DRM := true

BOARD_FLASH_BLOCK_SIZE := 524288
TARGET_USERIMAGES_USE_EXT4 := true

# WiFi settings
WPA_SUPPLICANT_VERSION := VER_0_8_X
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_HOSTAPD_DRIVER := NL80211

# BT configs
BOARD_HAVE_BLUETOOTH := true
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/rockchip/$(TARGET_DEVICE)/hal/bluetooth

# enable Treble
PRODUCT_FULL_TREBLE_OVERRIDE := true
BOARD_VNDK_VERSION := current
# Enable system property split for Treble
BOARD_PROPERTY_OVERRIDES_SPLIT_ENABLED := true

# SELinux policy dirs
PRODUCT_PUBLIC_SEPOLICY_DIRS  += device/rockchip/common/sepolicy/public
PRODUCT_PRIVATE_SEPOLICY_DIRS += device/rockchip/common/sepolicy/private
BOARD_VENDOR_SEPOLICY_DIRS    += device/rockchip/common/sepolicy/vendor

PRODUCT_PUBLIC_SEPOLICY_DIRS  += device/rockchip/$(TARGET_DEVICE)/sepolicy/public
PRODUCT_PRIVATE_SEPOLICY_DIRS += device/rockchip/$(TARGET_DEVICE)/sepolicy/private
BOARD_VENDOR_SEPOLICY_DIRS    += device/rockchip/$(TARGET_DEVICE)/sepolicy/vendor

# Android generic system image always create metadata partition
BOARD_USES_METADATA_PARTITION := true

# Set overlays
DEVICE_PACKAGE_OVERLAYS += device/rockchip/common/overlay
DEVICE_PACKAGE_OVERLAYS += device/rockchip/$(TARGET_DEVICE)/overlay
