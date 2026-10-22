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
