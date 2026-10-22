# Avaota F2 OV13850 适配补丁

基于 **Tina V861 v1.1 / Avaota F2 / Linux 6.6 Xuantie** 的 OV13850 摄像头最小适配，支持 **2560×1440 @ 30/60fps、MIPI CSI-2 四 Lane、RAW10 BGGR**，并接入原厂 ISP610 参数。

已验证 2K30/60 收帧和出图。这里的 **2K 指 2560×1440**。当前采用单路 TDM 离线通路，**未开启图像拼接**。

## 仓库内容

```text
source/                 修改后的完整文件，保留 SDK 相对路径，共 11 个
patches/                相对未适配 SDK 的完整差异补丁
test/                   2K30 / 2K60 单路采集配置
docs/changes.md         修改文件及用途
docs/validation.md      测试结果和已知限制
source-manifest.json    11 个修改文件的 SHA-256
SHA256SUMS              source/、patches/、test/ 文件校验值
```

本仓库提供 SDK 源文件和补丁，不包含完整 SDK、预编译固件或采集程序。

## 使用方法一：直接替换文件（推荐）

适用于与本仓库基线一致的 Tina V861 v1.1 SDK、Avaota F2 NOR 方案。其他版本或已有定制的 SDK 请先对比差异；覆盖前保存自己的修改，尤其是设备树和 defconfig。

```bash
# 根据实际位置修改这两个路径
PATCH_DIR="$HOME/Downloads/avaota-f2-ov13850-patches"
SDK_DIR="$HOME/tina-v861-v1.1"

# 已经克隆过仓库时跳过这一行
git clone https://github.com/dshanpi/avaota-f2-ov13850-patches.git "$PATCH_DIR"

cd "$PATCH_DIR"
sha256sum -c SHA256SUMS

# 将 source 里面的 bsp、device、openwrt、platform 合并到 SDK 同名目录
cp -a "$PATCH_DIR/source/." "$SDK_DIR/"
```

**复制 `source/` 里面的内容到 SDK 根目录，不是把整个 `source` 文件夹放进 SDK。** 这些文件用于替换 SDK 中的源码；复制完成后必须重新编译。

## 使用方法二：应用完整补丁

与方法一二选一，**不要重复应用**。在尚未适配 OV13850 的同版本 SDK 根目录执行：

```bash
cd "$SDK_DIR"
git apply --check "$PATCH_DIR/patches/ov13850-2k-with-vendor-isp.patch"
git apply "$PATCH_DIR/patches/ov13850-2k-with-vendor-isp.patch"
```

`git apply` 不要求 SDK 本身是 Git 仓库。若检查失败，先确认 SDK 版本和现有修改，不要强行套用。该补丁同时包含 Sensor、板级配置和原厂 ISP 接入。

## 编译

首次配置使用：

```bash
cd "$SDK_DIR"
./build.sh autoconfig -i v861 -o openwrt -b avaota_f2 -n nor -k linux-6.6-xuantie -a riscv32
./build.sh loadconfig

cp openwrt/target/v861/v861-avaota_f2/defconfig openwrt/openwrt/.config
./build.sh openwrt_rootfs defconfig
./build.sh
./build.sh pack
```

如果 SDK 已正确配置，可以从复制 OpenWrt `defconfig` 开始。构建使用 SDK 的 `./build.sh`，无需在非交互 shell 中依赖 `source build/envsetup.sh` 定义的函数。

固件打包输出可在 SDK 的 `out/` 下查找；本适配使用 Avaota F2 NOR 目标，标准镜像名为 `v861_linux_avaota_f2_uart0_nor.img`。首次安装驱动和设备树时，需用匹配的烧录工具烧录新镜像。

### 原厂 ISP 参数如何生效

本 SDK 使用静态 ISP 库。原厂参数编译到 `libisp_ini.a`，随后链接进采集或编码程序。

- 修改过 ISP 后，需要重新编译 ISP 库，并重新链接使用它的业务程序。
- 只替换 `ov13850_mipi.ko`，或把原厂 `.h` 复制到板端，不会启用新 ISP。
- 固件默认不将 MPP 采集示例安装到 rootfs，需要另外传入编译后的程序。

只重新编译 ISP 和 MPP 示例时：

```bash
cd "$SDK_DIR"
./build.sh openwrt_rootfs package/subpackage/allwinner/vision/libAWIspApi/compile V=s
./build.sh openwrt_rootfs package/subpackage/allwinner/eyesee-mpp/middleware/compile V=s
```

如果自有业务工程使用预编译静态库，请更新其链接输入并重新链接，不能继续运行旧二进制。

## 上板采集

新固件启动时由 `S00mpp` 自动加载 `ov13850_mipi`。驱动**不需要额外的 `insmod` 参数**；30/60fps 由采集程序在开流前选择，默认模式为 30fps。

使用 SDK 编译生成的 `sample_virvi` 和本仓库配置进行验证：

```bash
# 主机上执行；存在多块板时给 adb 指定正确的设备
adb push "$SDK_DIR/platform/allwinner/eyesee-mpp/middleware/sun252iw1/sample/sample_virvi/sample_virvi" /tmp/sample_virvi
adb push "$PATCH_DIR/test/isp-30.conf" /tmp/isp-test.conf
adb shell 'mkdir -p /tmp/minimal2k; chmod +x /tmp/sample_virvi'
adb shell 'LD_LIBRARY_PATH=/usr/lib/eyesee-mpp:/usr/lib /tmp/sample_virvi -path /tmp/isp-test.conf'
adb pull /tmp/minimal2k/frame.nv21 ./ov13850-2k30.nv21
```

测试 60fps 时改用 `test/isp-60.conf`。每轮运行前先保存上一轮图片；`/tmp` 是内存文件系统，重启后内容消失。输出为 NV21，2560×1440 单帧应为 **5,529,600 字节**。示例保存的是启动阶段的一帧，严格画质评估应等待 AE/AWB 收敛后再取图。

在运行日志中确认出现：

```text
find ov13850_mipi_2560_1440_30_0 [ov13850_mipi_isp610_20261021_195907] isp config
```

60fps 应匹配 `_60_0`。采集过程中 `/sys/kernel/debug/mpp/isp` 也应显示相同 ISP 配置名。

## 硬件与范围

- 测试模组：DshanPI_CAM-OV13850-V1.0，经转接板连接 Avaota F2 四 Lane 接口。
- 使用 MIPI0 / CSI0、TWI1、MCLK1；RESET 为 PA14，模组外部 3.3V 供电。
- SDK 中 I²C 地址为 `0x20`（8 位表示），Linux 7 位地址为 `0x10`。
- 设备树包含具体板级供电、引脚和路由，不适用于未经核对的其他转接板。
- 仅提供 1440p30/60；本仓库不包含 4K、1080p、720p 模式。

## 为什么仍然启用 TDM

本版 SDK 在 V861 四 Lane 模式下，只有启用 `CONFIG_SUPPORT_ISP_TDM` 才使用 MIPI `TWO_DATA` 输出。当前 Sensor 时序关闭该选项时，实测出现 MIPI / 行宽错误，未正常收帧；启用标准 TDM 离线配置后恢复。

通路为 `OV13850 → MIPI0 → CSI0 → TDM_RX0 → ISP0 → VIPP0`。Sensor 输入和最终输出均为 2560×1440，采集配置 `stitch_mode_0=0`、`dma_merge_scaler_en_0=0`。**TDM 离线通路不等于图像拼接，也不意味着所有 2K 配置都必须使用 TDM。**

## 验证状态

2026-10-22 白色天花板场景短测：2K30 约 **29.97fps**，2K60 约 **59.89fps**，均能获取完整图像；新 ISP 相比此前临时使用的 GC2083 参数，明显改善绿偏和噪声。

原厂参数标注为 1440p60；1440p30 共用同一套参数。短时存图仍有少量丢帧，且存在原厂参数与 SDK 的 AWB 字段兼容性告警。具体结果和边界见 [验证记录](docs/validation.md)。本仓库不宣称已完成零丢帧编码、长时间录制或完整 ISP 画质验收。
