# 修改文件

相对未适配的 SDK 共 20 个文件；`source/` 保存修改后的完整文件，补丁与这些文件一致。

## `bsp/drivers/vin/modules/sensor/ov13850_mipi.c`

OV13850 R2A 初始化、2560×1440@30/60、3840×2160@25、1920×1080@30/60、1280×720@30/60 时序、固定 VTS 曝光上限、芯片 ID 检查和开停流。新增模式复用此前验证过的寄存器表；本轮保留原有 2K 表不变。

## `device/config/chips/v861/configs/avaota_f2/linux-6.6-xuantie/board.dts`

Sensor 名称、地址、供电和引脚；TDM/ISP/VIPP 离线通路。第二逻辑 ISP/VIPP 通路为 4K 单帧分块处理启用，并连接同一 MIPI0/CSI0/Sensor0；2K/1080p/720p 使用第一逻辑通路且不拼接。SDC0 删除 `cd-gpios`，增加 `broken-cd`，以标准轮询替代 PF6 卡检测。

## `device/config/chips/v861/configs/avaota_f2/linux-6.6-xuantie/bsp_defconfig`

启用 OV13850 Sensor 模块和 CONFIG_SUPPORT_ISP_TDM。本轮增加 CONFIG_TDM_LBC_EN 和 CONFIG_FRAMEDONE_TWO_BUFFER，使用 SDK 现有 RAW 压缩及双缓冲配置，让 4K 在原 50MiB 视频池内运行。该 SDK 双缓冲开关实际对其他模式也生效，见实测记录。

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

加入 OV13850 参数头文件和七个尺寸/帧率匹配项；所有模式复用原厂参数，未修改解析算法。

## `platform/allwinner/vision/libAWIspApi/isp_mpp/isp_v861/libisp/isp_cfg/tina.mk`

增加 SENSOR_OV13850 条件编译宏。

未修改 VIN、TDM、VIPP、ISP 内核公共代码、CCI 或内存分配器。原板级 50MiB 内存池保留。

Sensor 时序为 MCLK 24MHz、HTS 4800。720p/1080p/2K 时序像素时钟 480MHz，30fps VTS 3333，60fps VTS 1667；4K 时序像素时钟约 342.857MHz、VTS 2857，对应约 25fps。时序计算值须与板端实测区分。
摄像头配置为非 WDR、四 Lane RAW10 BGGR；无 `lanes`、`video_modes`、`mode4k` 等模块参数。

原厂头文件 SHA-256：`752f97c6725d2f27b0fc5cdd871cc6219d224564012cd52f966b7e1d1d43adb5`。

## `openwrt/target/v861/v861-avaota_f2/busybox-init-base-files/etc/init.d/S05mdev`

新增六行板级启动脚本，启动 mdev 守护进程并扫描当前设备；复用 SDK 的 mdev.conf 和挂载脚本。

## `openwrt/package/allwinner/system/busybox-init-base-files/files/usr/bin/hotplug.sh`

仅增加已挂载判断和一行注释，防止重复 add 事件触发重复挂载。未改动其文件系统分支。

SD 部分相对 `b61eff6` 只涉及四个文件（两个已有摄像头板级文件、两个新增到本仓库的文件），
未修改 MMC 公共驱动或摄像头时序。详情见 [SD 卡说明](sd-card.md)。

## 相对 ca43209 的多模式增量

仅上述 Sensor、board.dts、bsp_defconfig、isp_ini_parse.c 四个 SDK 源文件改变。其余九个文件保持字节一致，SD 逻辑和原厂 ISP 头文件未改。完整补丁为 `patches/avaota-f2-ov13850-sd.patch`；四文件增量为 `patches/avaota-f2-ov13850-multimode-from-ca43209.patch`。

`test/sample_virvi-multimode.patch` 是独立的应用示例补丁，不在 `source/` 或四文件增量中：4K 请求两个 YUV 缓冲；720p 在 ISP 启动前使用现有 `AW_MPI_ISP_SetD3dLbcRatio(isp, 100)` 接口。没有引入新的模块参数或内核公共代码补丁。

## TF 接口 WiFi 增量

相对 v0.1.0，本轮另外适配 RTL8189FS：六个已有驱动文件的 RISC-V/Linux 6.6 兼容、board.dts 的 SDIO 与 auto_print、BSP 和 OpenWrt defconfig、modules.mk，以及新的六行 S06tfwifi 启动脚本，共 11 个文件、391 行增量补丁。与摄像头部分共同维护的文件没有另建重复副本。所有详细路径见 [TF WiFi 说明](tf-wifi.md)。

WiFi 模块位于 NOR rootfs，不依赖 TF 卡文件系统。模块每次开机注册，内核按 SDIO ID 匹配；普通 TF 卡仍复用 S05mdev 和原有挂载脚本。相机驱动及 ISP 文件在本轮没有变化。没有修改 MMC 公共驱动或分区表。
