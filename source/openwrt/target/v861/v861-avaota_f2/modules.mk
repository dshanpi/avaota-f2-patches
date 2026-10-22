#
# Copyright (C) 2015-2016 Allwinner
#
# This is free software, licensed under the GNU General Public License v2.
# See /build/LICENSE for more information.

define KernelPackage/sensor-ov13850
	SUBMENU:=$(VIDEO_MENU)
	TITLE:=Sensor ov13850 support
	DEPENDS:= +kmod-vin-v4l2
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/vin/modules/sensor/ov13850_mipi.ko
	AUTOLOAD:=$(call AutoProbe,ov13850_mipi)
endef

$(eval $(call KernelPackage,sensor-ov13850))

# Use BSP-native cfg80211 and RTL8189FS from this kernel, not backports.
define KernelPackage/tf-rtl8189fs
	SUBMENU:=$(WIRELESS_MENU)
	TITLE:=Avaota F2 removable TF RTL8189FS SDIO WiFi
	KCONFIG:=CONFIG_CFG80211=m CONFIG_CFG80211_WEXT=y CONFIG_RTL8189FS=m
	FILES:= \
		$(CONFIG_EXTERNAL_KERNEL_TREE)/net/wireless/cfg80211.ko \
		$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/net/wireless/rtl8189fs/8189fs.ko
endef

$(eval $(call KernelPackage,tf-rtl8189fs))
