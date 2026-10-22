# 修改文件

相对未适配的 SDK 共 13 个文件；`source/` 保存修改后的完整文件，补丁与这些文件一致。

## `bsp/drivers/vin/modules/sensor/ov13850_mipi.c`

OV13850 R2A 初始化、2560×1440@30/60 时序、固定 VTS 曝光上限、芯片 ID 检查和开停流。

## `device/config/chips/v861/configs/avaota_f2/linux-6.6-xuantie/board.dts`

Sensor 名称、地址、供电和引脚；单路 TDM/ISP/VIPP 离线通路，第二逻辑通路禁用。SDC0 删除 `cd-gpios`，增加 `broken-cd`，以标准轮询替代 PF6 卡检测。

## `device/config/chips/v861/configs/avaota_f2/linux-6.6-xuantie/bsp_defconfig`

启用 OV13850 Sensor 模块和 CONFIG_SUPPORT_ISP_TDM。

## `openwrt/package/allwinner/vision/libAWIspApi/machinfo/v861/Config.in`

新增 OV13850 ISP 参数选择项。

## `openwrt/package/allwinner/vision/libAWIspApi/machinfo/v861/libisp_config.mk`

将 CONFIG_ov13850 映射为 ISP 构建使用的 Sensor 名称。

## `openwrt/target/v861/v861-avaota_f2/busybox-init-base-files/etc/init.d/S00mpp`

在 vin_io 之后、vin_v4l2 之前加载 OV13850 模块。

## `openwrt/target/v861/v861-avaota_f2/defconfig`

选择 OV13850 模块和 ISP 参数，取消 GC2083 对应选择。启用 BusyBox mdev、配置规则、脚本执行及 netlink daemon 四项功能。

## `openwrt/target/v861/v861-avaota_f2/modules.mk`

本板新增 OV13850 内核模块打包规则。

## `platform/allwinner/vision/libAWIspApi/isp_mpp/isp_v861/libisp/isp_cfg/SENSOR_H/ov13850/ov13850_mipi_isp610_20261021_195907.h`

原厂 ISP610 参数，按字节原样保存。

## `platform/allwinner/vision/libAWIspApi/isp_mpp/isp_v861/libisp/isp_cfg/isp_ini_parse.c`

加入 OV13850 参数头文件和 30/60fps 匹配项；未修改解析算法。

## `platform/allwinner/vision/libAWIspApi/isp_mpp/isp_v861/libisp/isp_cfg/tina.mk`

增加 SENSOR_OV13850 条件编译宏。

未修改 VIN、TDM、VIPP、ISP 内核公共代码、CCI 或内存分配器。原板级 50MiB 内存池保留。

Sensor 时序为 MCLK 24MHz、HTS 4800、时序像素时钟 480MHz；30fps VTS 3333，60fps VTS 1667。
摄像头配置为非 WDR、四 Lane RAW10 BGGR；无 `lanes`、`video_modes`、`mode4k` 等模块参数。

原厂头文件 SHA-256：`752f97c6725d2f27b0fc5cdd871cc6219d224564012cd52f966b7e1d1d43adb5`。

## `openwrt/target/v861/v861-avaota_f2/busybox-init-base-files/etc/init.d/S05mdev`

新增六行板级启动脚本，启动 mdev 守护进程并扫描当前设备；复用 SDK 的 mdev.conf 和挂载脚本。

## `openwrt/package/allwinner/system/busybox-init-base-files/files/usr/bin/hotplug.sh`

仅增加已挂载判断和一行注释，防止重复 add 事件触发重复挂载。未改动其文件系统分支。

SD 部分相对 `b61eff6` 只涉及四个文件（两个已有摄像头板级文件、两个新增到本仓库的文件），
未修改 MMC 公共驱动或摄像头时序。详情见 [SD 卡说明](sd-card.md)。
