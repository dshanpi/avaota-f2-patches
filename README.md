# Avaota F2 适配补丁

基于 **Tina V861 v1.1 / Avaota F2 NOR / Linux 6.6 Xuantie**，提供 OV13850 四 Lane RAW10 BGGR 驱动、原厂 ISP610 参数接入，以及 TF 卡自动识别和挂载。

| 模式 | Sensor 输出 | 帧率 | 采集通路 |
|---|---|---|---|
| 720p | 1280×720 | 30 / 60fps | video0 / ISP0，不拼接 |
| 1080p | 1920×1080 | 30 / 60fps | video0 / ISP0，不拼接 |
| 2K / 1440p | 2560×1440 | 30 / 60fps | video0 / ISP0，不拼接 |
| 4K | 3840×2160 | 25fps | video1 / ISP1，单帧左右分块处理再合并 |

这是 Sensor 原生模式选择，不是把 2K 图像缩放成其他尺寸。720p/1080p 使用 Sensor 裁剪，视场与 2K/4K 不同。默认仍为 2K30，**没有 `lanes`、`video_modes`、`mode4k` 等模块参数**；应用在开流前指定尺寸与帧率。

本轮实测结果和限制见 [多模式验证记录](docs/multimode-validation.md)。采集帧率不代表编码帧率，原厂 ISP 参数复用不代表各分辨率均完成了独立画质标定。

## 仓库内容与修改范围

```text
source/                 13 个完整源文件，保留 SDK 相对路径
patches/                原始 SDK 完整补丁和指定版本增量补丁
test/modes/             7 档模式，各一份存图和一份持续收帧配置
test/sample_virvi-multimode.patch  4K/720p 采集示例的独立补丁
test/                   保留原有 2K 测试配置
docs/changes.md         每个修改文件的用途
docs/multimode-validation.md      本轮多模式测试记录
docs/clean-sdk-with-sd-validation.md  先前 2K + TF 卡干净 SDK 复现
source-manifest.json    13 个源文件的 SHA-256
SHA256SUMS              source/、patches/、test/ 校验值
```

相对已验证的 2K+TF 卡基线 `ca43209`，只改四个 SDK 文件：Sensor 模式表、ISP 配置匹配表、板级设备树、BSP defconfig。原厂 ISP 头文件和原有 2K 寄存器表保持不变，保留原板级 50MiB 视频内存池；没有修改 VIN/TDM/VIPP/ISP 内核公共实现、CCI 或内存分配器。

4K/720p 另需采集程序的两处配置，见下文。仓库不包含完整 SDK、预编译固件或采集程序。

## 安装到 SDK

适用于相同版本 SDK 和 Avaota F2 NOR 板级方案。覆盖前保存自己的修改，尤其是设备树和 defconfig。

```bash
PATCH_DIR="$HOME/Downloads/avaota-f2-patches"
SDK_DIR="$HOME/tina-v861-v1.1"
# 已克隆时跳过；也可使用 git@github.com:dshanpi/avaota-f2-patches.git
git clone https://github.com/dshanpi/avaota-f2-patches.git "$PATCH_DIR"
cd "$PATCH_DIR"
sha256sum -c SHA256SUMS
cp -a "$PATCH_DIR/source/." "$SDK_DIR/"
```

复制的是 `source/` 的内容到 SDK 根目录。必须重新编译，不是将这些源码复制到开发板。

也可以在未适配的原始 SDK 上应用完整补丁，**与文件替换二选一**：

```bash
cd "$SDK_DIR"
git apply --check "$PATCH_DIR/patches/avaota-f2-ov13850-sd.patch"
git apply "$PATCH_DIR/patches/avaota-f2-ov13850-sd.patch"
```

若 SDK 已完整应用 `ca43209` 的 2K+TF 卡配置，则可用以下四文件增量补丁代替完整补丁：

```bash
cd "$SDK_DIR"
git apply --check "$PATCH_DIR/patches/avaota-f2-ov13850-multimode-from-ca43209.patch"
git apply "$PATCH_DIR/patches/avaota-f2-ov13850-multimode-from-ca43209.patch"
```

`git apply` 不要求 SDK 本身是 Git 仓库。检查失败时先核对版本和已有修改，不要强行套用或重复应用。旧 `avaota-f2-sd-poll-automount.patch` 仅供 `b61eff6` 增加 SD 支持，不能用它升级到多模式版本。

## 编译固件与采集程序

宿主机需先按 SDK 文档安装编译依赖。本轮使用 Ubuntu 24.04.4 LTS。首次运行 `./build.sh` 时，需在终端阅读 SDK 声明并按提示确认。

需要测试 4K/720p 时，**在编译前**应用独立的示例程序补丁：

```bash
cd "$SDK_DIR"
git apply --check "$PATCH_DIR/test/sample_virvi-multimode.patch"
git apply "$PATCH_DIR/test/sample_virvi-multimode.patch"
```

它使 4K 请求两个 YUV 缓冲区，并在 720p 的 `AW_MPI_ISP_Run` 前调用 `AW_MPI_ISP_SetD3dLbcRatio(isp, 100)`，避免此前复现的 D3D 压缩故障。其他尺寸保留示例的三个缓冲区请求，但本 SDK 开启 `CONFIG_FRAMEDONE_TWO_BUFFER` 后，驱动实际统一分配两个，需以运行日志为准。自有应用也需要对应配置；这些是 SDK 现有 API，不是模块参数。仅测试 2K/1080p 可使用原版示例。

```bash
cd "$SDK_DIR"
./build.sh autoconfig -i v861 -o openwrt -b avaota_f2 -n nor -k linux-6.6-xuantie -a riscv32
./build.sh loadconfig
cp openwrt/target/v861/v861-avaota_f2/defconfig openwrt/openwrt/.config
./build.sh openwrt_rootfs defconfig
./build.sh
./build.sh pack
```

烧录 `out/v861_linux_avaota_f2_uart0_nor.img`。首次应用设备树/内核配置需要完整新固件。若经 Windows 手工复制文件，还需确认 `S05mdev` 保留执行权限。

ISP 参数静态链接进业务程序：只替换 `ov13850_mipi.ko` 或把 `.h` 传到板端不会更新 ISP。固件默认不安装 MPP 示例，需要单独传输。只重新编译 ISP 和示例可执行：

```bash
./build.sh openwrt_rootfs package/subpackage/allwinner/vision/libAWIspApi/compile V=s
./build.sh openwrt_rootfs package/subpackage/allwinner/eyesee-mpp/middleware/compile V=s
```

## 上板采集

启动脚本自动加载驱动。以下以 1080p60 为例，测试其他模式只改 `MODE`：`720p30`、`720p60`、`1080p30`、`1080p60`、`1440p30`、`1440p60`、`4k25`。

```bash
MODE=1080p60
# 多块板时，先用 adb -s 或 -t 选择目标板；下面假定只有一个目标设备
adb shell 'grep " /mnt/SDCARD " /proc/mounts'
adb shell 'mkdir -p /mnt/SDCARD/ov13850-multimode-current'
adb push "$SDK_DIR/platform/allwinner/eyesee-mpp/middleware/sun252iw1/sample/sample_virvi/sample_virvi" /mnt/SDCARD/sample_virvi
adb push "$PATCH_DIR/test/modes/$MODE-save.conf" /tmp/capture.conf
adb shell 'chmod +x /mnt/SDCARD/sample_virvi'
adb shell 'LD_LIBRARY_PATH=/usr/lib/eyesee-mpp:/usr/lib /mnt/SDCARD/sample_virvi -path /tmp/capture.conf'
adb pull /mnt/SDCARD/ov13850-multimode-current/frame.nv21 "./$MODE.nv21"
```

先确认 TF 卡已挂载；每轮先取回上一张图片，配置会覆盖同一路径。输出是 NV21，大小为 `宽×高×3/2`，可用对应尺寸的 YUV 播放器查看。测试程序运行 35 秒，开头丢弃 90 帧，再保存一帧；这不构成严格 AE/AWB 收敛或画质验收。

日志应匹配所选模式，例如：

```text
find ov13850_mipi_1920_1080_60_0 [ov13850_mipi_isp610_20261021_195907] isp config
```

4K 配置使用 `dev_num_0=1`、`isp_dev_num_0=1`、`stitch_mode_0=1`；其他模式为 `0/0/0`。一次只运行一个模式，停止后再切换。

### 持续收帧和实际帧率

`*-save.conf` 用来获取图片。原版示例在存满一张后仍执行整帧复制和清零，会引入 CPU/内存开销及丢帧。测持续收帧应使用关闭存图的 `*-stream.conf`：

```bash
adb push "$PATCH_DIR/test/modes/$MODE-stream.conf" /tmp/capture.conf
adb shell 'LD_LIBRARY_PATH=/usr/lib/eyesee-mpp:/usr/lib /mnt/SDCARD/sample_virvi -path /tmp/capture.conf'
```

在另一个终端于开流约 10 秒后，间隔 20 秒采集两次：

```bash
adb shell 'cat /proc/uptime; cat /sys/kernel/debug/mpp/vi; cat /proc/uptime' > vi-a.txt
sleep 20
adb shell 'cat /proc/uptime; cat /sys/kernel/debug/mpp/vi; cat /proc/uptime' > vi-b.txt
```

选取活动 `vi0`（4K 为 `vi1`）的 `frame => cnt` 差值，除以 uptime 差值；每个文件 uptime 取首末行第一列均值。同时比较 `lost_cnt`、`error_cnt` 增量，并检查输入、输出尺寸。VI 硬件计数不等于应用交付或编码帧率。

## TDM 与 4K 分块

本 SDK 的四 Lane 接收通路启用 `CONFIG_SUPPORT_ISP_TDM` 后使用 MIPI `TWO_DATA` 输出；此前关闭后出现行宽/MIPI 错误。沿用已验证的 TDM 离线配置。

720p、1080p、2K 使用 `OV13850 → MIPI0 → CSI0 → TDM_RX0 → ISP0 → VIPP0`，不启用拼接。4K 使用 SDK 自带的左右分块处理和合并，处理的是同一颗 Sensor 的同一帧，不是双摄拼接。为 4K 启用第二逻辑 ISP/VIPP 路由、`CONFIG_TDM_LBC_EN` 和 `CONFIG_FRAMEDONE_TWO_BUFFER`；没有改公共驱动实现，也没有扩大内存池。

## TF 卡

MMC 使用轮询检测，BusyBox mdev 复用 SDK 挂载脚本。FAT32 单分区卡挂载到 `/mnt/SDCARD`；可用 `grep mmc /proc/mounts`、`df -h /mnt/SDCARD` 检查。

拔卡前先停止采集和写入，再执行 `sync; umount /mnt/SDCARD`。自动识别不代表写入时可直接拔卡。轮询无卡时可能出现 MMC 超时日志。详情见 [SD 卡说明](docs/sd-card.md)。

## 硬件和已知边界

- DshanPI_CAM-OV13850-V1.0，经转接板接 Avaota F2 四 Lane 接口；MIPI0/CSI0、TWI1、MCLK1、RESET PA14、模组外部 3.3V。
- SDK I²C 地址 `0x20` 为 8 位表示，Linux 7 位地址为 `0x10`。其他模组/转接板须核对引脚。
- 原厂 ISP 参数标注为 1440p60，所有模式复用同一套参数；头文件按字节原样保留。
- 原厂参数与 SDK 的 AWB 字段兼容性告警、偶发 I²C 重试需按日志如实记录；短时出图不代表这些告警均已解决。
- 4K 两次测试均在停流边界出现一次 `tdm0 rx0 frame error`，持续收帧统计窗口内没有帧错误。此停流告警仍保留，不能将本版描述为所有阶段零报错。
- 本仓库不宣称完成各模式的长期零丢帧编码、全部场景画质验收或对不同硬件版本的兼容性认证。

先前 2K-only 版本的干净 SDK 全量复现记录见 [历史验证](docs/clean-sdk-with-sd-validation.md)，不能替代本轮新增模式验证。
