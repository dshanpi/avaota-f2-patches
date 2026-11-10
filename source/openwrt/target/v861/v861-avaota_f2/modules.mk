#
# Copyright (C) 2015-2016 Allwinner
#
# This is free software, licensed under the GNU General Public License v2.
# See /build/LICENSE for more information.

define KernelPackage/sensor-gc2093
	SUBMENU:=$(VIDEO_MENU)
	TITLE:=Sensor GC2093 support
	DEPENDS:= +kmod-vin-v4l2
	FILES+=$(CONFIG_EXTERNAL_KERNEL_TREE)/bsp/drivers/vin/modules/sensor/gc2093_mipi.ko
	AUTOLOAD:=$(call AutoProbe,gc2093_mipi)
endef

$(eval $(call KernelPackage,sensor-gc2093))
