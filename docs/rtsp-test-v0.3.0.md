# OV13850 RTSP 测试与源码编译 v0.3.0

适用于 Avaota F2 NOR、Tina V861 v1.1、OV13850 四 Lane、Linux 6.6 Xuantie。推荐 1080P30/2K30；**4K 目标 25fps 本轮只测得约 15.9fps，存在持续 overwrite 和秒级停顿，未通过运行验收。**

## 一、下载和校验

下载 Release 的 `SHA256SUMS`、配套 `.img`、`ov13850-rtsp-app-v0.3.0.tar`，以及 `WIFI_CONNECT.md`。Ubuntu 主机在附件所在目录执行：

```sh
sha256sum -c SHA256SUMS --ignore-missing
```

此命令只检查已下载的附件；完整下载全部附件后可去掉 `--ignore-missing`。Windows 也可用 `Get-FileHash 文件名 -Algorithm SHA256` 与清单对照。

镜像沿用 v0.2.0 的 `v861_avaota_f2_tf_wifi_network_20261023.img`，不是新编译固件。已经运行此镜像的板子只需更新应用包。其他固件请先按开发板/LYNX 的烧录方法完整烧录配套镜像；不要使用 GC2093 或其他板型镜像代替。镜像及本轮板端的 boot、rootfs、三个关键驱动模块哈希已核对，见 `FIRMWARE_INFO.json`。

## 二、WiFi 和运行包

断电插入 TF 接口 RTL8189FS WiFi 卡，上电后通过串口确认 `ifconfig -a` 中出现 wlan0。按 `WIFI_CONNECT.md` 设置自己的 SSID/密码，等待 WPA2 认证完成，再通过 DHCP 获取地址；本包没有开机自动连接路由器服务。

应用包内含原版 SDK 的 RISC-V 32 位 `sample_smartIPC_demo`、RTSP 修正库、三档配置、两个 ISP bin、启停脚本及内部校验清单。只传运行 tar，不要把完整源码或视频传入板端有限的 `/tmp`。

主机端 ADB 仅用于文件传输。有多台设备时先用 `adb devices -l` 选择正确板子，并为 push 加 `-s`：

```sh
adb push ov13850-rtsp-app-v0.3.0.tar /tmp/ov13850-runtime.tar
```

以下命令均在**开发板串口**运行。先停止已有摄像头程序，等待其退出；传输、解包建议预留约 15MB 的 `/tmp` 空间：

```sh
df -h /tmp
mkdir -p /tmp/ov13850-display
tar -xf /tmp/ov13850-runtime.tar -C /tmp/ov13850-display
rm /tmp/ov13850-runtime.tar
cd /tmp/ov13850-display
sha256sum -c SHA256SUMS
ifconfig wlan0
sh /tmp/ov13850-display/run.sh 1440p30
```

BusyBox tar 不支持 `-z`，因此运行包使用无压缩 `.tar`，用 `tar -xf` 解包。应用启动前脚本加载对应 ISP 参数，并设置仅对子进程有效的 `LD_PRELOAD`，不要绕过脚本直接运行原应用来比较本包效果。

在 PC 用 VLC 打开 `rtsp://开发板wlan0的IP:8554/ch0`，选择 RTSP/TCP，可先设置 500～1000ms 网络缓存；建议只连接一个客户端。IP 使用自己的 DHCP 地址。

## 三、切换、限时和停止

运行终端按 Ctrl+C，等待退出完成，再选另一档：

```sh
sh /tmp/ov13850-display/run.sh 1080p30
```

默认持续运行，60 秒限时测试示例：

```sh
DURATION=60 sh /tmp/ov13850-display/run.sh 1440p30
```

如通过另一个串口终端停止：

```sh
sh /tmp/ov13850-display/stop.sh
```

`stop.sh` 发送停止信号后立即返回。等待原运行终端退出，或确认 `/tmp/ov13850-display.lock` 消失后再切换；旧进程仍在退出时，新启动会被拒绝。脚本退出恢复原有 `/tmp/isp_test/color/isp_param_config.bin`，原本不存在时删除本次副本。断电后 `/tmp` 清空，需要重新传应用；SIGKILL 不执行退出清理，须先核对是否有遗留测试进程。

`sh /tmp/ov13850-display/run.sh 4k25` 仅用于复现 **4K 实验模式**，不能用来宣称稳定 4K25。4K 使用固件既有的左右分块合并路径，1080P/2K 不拼接。不要同时运行多个档位。

## 四、PC 录像和 USB 对照

在装有 FFmpeg 的 PC 保存原始 H.265，不重复编码：

```sh
ffmpeg -rtsp_transport tcp -i rtsp://BOARD_IP:8554/ch0 -t 60 -an -c:v copy capture.mkv
ffmpeg -hide_banner -v error -ss 2 -err_detect explode -i capture.mkv \
  -map 0:v:0 -fps_mode passthrough -enc_time_base 1:1000 -f null -
```

第二条跳过连接最初 2 秒检查稳态解码。能解码不表示编码前没有丢帧；同时检查应用/内核的 `online-error: overwrite`、VE 复位和 encode error，以及实际帧间隔。

排除 WiFi 时，在 PC 执行：

```sh
adb forward tcp:18554 tcp:8554
```

在开发板串口运行 `NET_TYPE=0 sh /tmp/ov13850-display/run.sh 1440p30`，PC 播放 `rtsp://127.0.0.1:18554/ch0`。结束后停止应用，PC 执行 `adb forward --remove tcp:18554`。

## 五、完整源码编译

源码附件分为：

- `avaota-f2-patches-v0.3.0.zip`：完整仓库快照，含 21 个 SDK 适配源文件、补丁、参数和验证记录。
- `ov13850-rtsp-source-v0.3.0.tar.gz`：本次完整测试/应用源码，包括 `sample_smartIPC_demo.c/.h/_config.h`、公共示例辅助代码、配置解析器、sample 构建 makefile、RTSP 修正库、ISP 导出器、配置和启停脚本。原 SDK C/H 未修改。

需要匹配版本 Tina SDK、SDK 的 RISC-V 工具链与 MPP 库；源码包不包含完整 SDK 或厂商闭源库。先按仓库 README 将 `source/.` 覆盖到 SDK，并完成板级配置和固件构建。下面展示 SDK 已准备好的应用编译步骤：

```bash
SDK_DIR="$HOME/tina-v861-v1.1"
tar -xzf ov13850-rtsp-source-v0.3.0.tar.gz
cd ov13850-rtsp-source-v0.3.0
# 先备份自己的 SDK 改动；以下复制原版应用源码和公共示例辅助代码
cp -a sdk-source/. "$SDK_DIR/"

cd "$SDK_DIR"
./build.sh openwrt_rootfs package/subpackage/allwinner/vision/libAWIspApi/compile V=s
./build.sh openwrt_rootfs package/subpackage/allwinner/eyesee-mpp/middleware/compile V=s
```

回到解压的 `ov13850-rtsp-source-v0.3.0` 目录：

```bash
python3 rtsp-ov13850/build.py --sdk "$SDK_DIR"
cd rtsp-ov13850/build
sha256sum -c SHA256SUMS
```

Ubuntu x86_64 主机需有 `python3`、本机 `gcc` 和 SDK 原有依赖。构建脚本自动寻找 SDK 中的 `riscv32-unknown-linux-musl-gcc`；多个工具链时加 `--cross-cc /绝对路径/编译器`。只生成自己的 `build/`，不修改 SDK 的 ISP 头文件；`BUILD-INFO.json` 记录原应用哈希、ISP 头文件哈希和编译器。

输出 `rtsp-ov13850/build/ov13850-runtime.tar`，传入板端后按第二节解包运行。SDK 原应用 SHA-256 为 `f450f0cb19e613a5b667fa9260fd96613960de6d733a6726b77021cddf245cb0`；其他哈希的应用需要重新确认 ABI、符号、外部 ISP 参数加载和各模式，不能直接当作已验证。

本次源包生成的应用、修正库、两个 ISP bin、三个配置和两个脚本，均与上板实测文件逐字节一致。原厂 ISP 头的 AWB 数值转换警告保留，不是新做的 AWB 标定。

## 六、验收边界

一分钟复测：1080P30 约 30.007fps，2K30 约 30.010fps；4K 约 15.883fps，最长帧间隔约 3.902 秒，**未通过**。详细数据见 `VALIDATION.json`。本轮没有重新烧录镜像，而是核对待发镜像的 boot/rootfs 和关键模块与实测板一致。

4K 在线编码 overwrite、停流 TDM 告警、可选 ioctl 提示及 RTSP 初始 H.265 参数集顺序问题尚未修复。Windows VLC 2K 无界面软件解码能连接和解码，但仍有启动等待/迟帧提示。WiFi 的排队和抖动取决于链路，不能保证低延迟或零丢帧。

这不是长时间稳定性、运动压力、暗光或完整 ISP 标定验收。当前推荐将 1080P30、2K30 用于功能展示，4K 留作问题复现。实拍视频/照片和真实 WiFi 凭据没有发布。
