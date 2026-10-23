# Avaota F2 适配补丁

基于 **Tina V861 v1.1 / Avaota F2 NOR / Linux 6.6 Xuantie**，提供 OV13850 四 Lane RAW10 BGGR 驱动、原厂 ISP610 参数接入，以及 TF 卡自动识别/挂载、RTL8189FS TF 接口 WiFi 自动驱动加载。

| 模式 | Sensor 输出 | 帧率 | 采集通路 |
|---|---|---|---|
| 720p | 1280×720 | 30 / 60fps | video0 / ISP0，不拼接 |
| 1080p | 1920×1080 | 30 / 60fps | video0 / ISP0，不拼接 |
| 2K / 1440p | 2560×1440 | 30 / 60fps | video0 / ISP0，不拼接 |
| 4K | 3840×2160 | 25fps | video1 / ISP1，单帧左右分块处理再合并 |

这是 Sensor 原生模式选择，不是把 2K 图像缩放成其他尺寸。720p/1080p 使用 Sensor 裁剪，视场与 2K/4K 不同。默认仍为 2K30，**没有 `lanes`、`video_modes`、`mode4k` 等模块参数**；应用在开流前指定尺寸与帧率。

本轮实测结果和限制见 [多模式验证记录](docs/multimode-validation.md)。采集帧率不代表编码帧率，原厂 ISP 参数复用不代表各分辨率均完成了独立画质标定。

## TF 插槽 WiFi

新增 RTL8189FS TF 接口 SDIO WiFi：同一插槽使用普通 FAT32 存储卡时自动挂载，换成 WiFi 卡后上电自动加载匹配内核模块并创建无线接口。驱动在开机时注册，内核按卡类型绑定；无需手工 insmod。使用与验收步骤见 [TF 卡 WiFi 最小适配](docs/tf-wifi.md)。当前轮次的验证状态见 [干净 SDK 验证记录](docs/tf-wifi-validation.md)。

## 连接 WiFi 路由器、获取 IP

使用当前源码重新编译的 **TF WiFi 联网版**；此前仅验证 wlan0 的固件没有无线认证后端和 DHCP，不能直接使用以下步骤。联网版在原驱动适配上只额外修改两个文件：板级 OpenWrt defconfig 启用 `udhcpc`、`wpa_cli`；hostapd 包 Makefile 将原生 `kmod-tf-rtl8189fs` 接入 nl80211、libnl 依赖与链接规则。原生内核 cfg80211 继续使用，不引入另一套 backports 模块。

断电插入 RTL8189FS WiFi 卡并上电，通过串口执行。以下以普通 **2.4GHz WPA2-PSK** 路由器为例，替换示例中的名称和密码；不要把自己的配置提交到仓库。

```sh
ifconfig -a
ifconfig wlan0 up
iw dev wlan0 scan | grep -E 'SSID:|signal:|freq:'
# 确认 drivers 列表包含 nl80211
wpa_supplicant -h

umask 077
mkdir -p /tmp/wpa_supplicant
cat > /tmp/wpa_supplicant.conf <<'EOF'
ctrl_interface=/tmp/wpa_supplicant
update_config=0
network={
    ssid="YOUR_WIFI_SSID"
    psk="YOUR_WIFI_PASSWORD"
    key_mgmt=WPA-PSK
    proto=RSN
}
EOF

wpa_supplicant -B -D nl80211 -i wlan0 -c /tmp/wpa_supplicant.conf -P /tmp/wpa_supplicant.pid
wpa_cli -p /tmp/wpa_supplicant -i wlan0 status
```

等待状态变为 `wpa_state=COMPLETED` 后再获取 IP，认证尚未完成时稍等并重复查询。WPA2 密码通常为 8～63 个字符；名称或密码包含双引号等特殊字符时须按 wpa_supplicant 配置语法填写。不要同时启动多个 wpa_supplicant 或 DHCP 客户端。

```sh
udhcpc -i wlan0 -s /usr/share/udhcpc/default.script -p /tmp/udhcpc-wlan0.pid -n -t 5 -T 3
ifconfig wlan0
route -n
cat /etc/resolv.conf
wpa_cli -p /tmp/wpa_supplicant -i wlan0 status
```

`udhcpc` 成功取得租约后保持后台续租。应看到 wlan0 的 IPv4 地址、指向 wlan0 的默认网关和 DNS；实际地址由路由器分配，不要照抄本次测试地址。

```sh
# 替换成 route -n 中本次 DHCP 分配的网关
ping -I wlan0 -c 5 192.168.1.1
# 外网和 DNS，是否成功还取决于路由器的上网/防火墙设置
ping -I wlan0 -c 5 1.1.1.1
nslookup example.com
```

本轮已验证 WPA2 认证、DHCP、网关/局域网/外网 IP 通信、DNS，外网 HTTP，以及双向 TCP 文件校验，详见 [WiFi 联网实测](docs/wifi-network-validation.md)。这不是无线吞吐率或长时间稳定性认证。

配置放在 `/tmp`，断电后清除；本版自动加载驱动，连接路由器仍按上述步骤手动执行。需要保留配置时可将自己的配置以 600 权限保存到 `/etc/wpa_supplicant.conf`，启动时改用该路径；这不会自动增加开机连接服务。修改网络前先结束已有进程：

```sh
[ ! -f /tmp/udhcpc-wlan0.pid ] || kill "$(cat /tmp/udhcpc-wlan0.pid)"
wpa_cli -p /tmp/wpa_supplicant -i wlan0 terminate
```

如果已经完整应用 `b01d6da` 的驱动版，可用 `patches/avaota-f2-wifi-network-from-b01d6da.patch` 的两文件增量后重新编译并烧录；原始 SDK 仍按下文覆盖全部 21 个 source 文件或应用完整补丁。

## 预编译固件和测试应用

v0.1.0 为先前摄像头版本，**不含本次 TF WiFi 适配**。可以从 [v0.1.0 Release](https://github.com/dshanpi/avaota-f2-patches/releases/tag/v0.1.0) 下载匹配固件、采集应用和测试配置，使用 `run.sh <模式> save|stream` 存图或测持续收帧。完整步骤见 [Release 摄像头测试说明](docs/camera-test-v0.1.0.md)。

## 仓库内容与修改范围

```text
source/                 21 个完整源文件，保留 SDK 相对路径
patches/                原始 SDK 完整补丁和指定版本增量补丁
test/modes/             7 档模式，各一份存图和一份持续收帧配置
test/sample_virvi-multimode.patch  4K/720p 采集示例的独立补丁
test/                   保留原有 2K 测试配置
docs/changes.md         每个修改文件的用途
docs/multimode-validation.md      本轮多模式测试记录
docs/clean-sdk-with-sd-validation.md  先前 2K + TF 卡干净 SDK 复现
source-manifest.json    21 个源文件的 SHA-256
SHA256SUMS              source/、patches/、test/ 校验值
```

摄像头多模式部分相对已验证的 2K+TF 卡基线 `ca43209`，只改四个 SDK 文件：Sensor 模式表、ISP 配置匹配表、板级设备树、BSP defconfig。原厂 ISP 头文件和原有 2K 寄存器表保持不变，保留原板级 50MiB 视频内存池；没有修改 VIN/TDM/VIPP/ISP 内核公共实现、CCI 或内存分配器。

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
git apply --check "$PATCH_DIR/patches/avaota-f2-ov13850-sd-wifi.patch"
git apply "$PATCH_DIR/patches/avaota-f2-ov13850-sd-wifi.patch"
```

若 SDK 已完整应用 `v0.1.0`，可使用 `patches/avaota-f2-tf-wifi-from-v0.1.0.patch` 的 12 文件增量，具体命令见 [TF WiFi 说明](docs/tf-wifi.md)。文件替换、完整补丁、对应版本增量补丁择一，不要重复应用。旧的 SD-only 和 ca43209 多模式增量保留作历史参考，不包含本次 WiFi 适配。

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

烧录 `out/v861_linux_avaota_f2_uart0_nor.img`。首次应用设备树/内核配置需要完整新固件。若经 Windows 手工复制文件，还需确认 `S05mdev`、`S06tfwifi` 保留执行权限。

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

当前 `source/` 另包含 TF WiFi 的 12 文件增量，详情见 TF WiFi 文档；其中设备树、BSP defconfig、OpenWrt defconfig、modules.mk 与摄像头/SD 配置共同维护。

## 硬件和已知边界

- DshanPI_CAM-OV13850-V1.0，经转接板接 Avaota F2 四 Lane 接口；MIPI0/CSI0、TWI1、MCLK1、RESET PA14、模组外部 3.3V。
- SDK I²C 地址 `0x20` 为 8 位表示，Linux 7 位地址为 `0x10`。其他模组/转接板须核对引脚。
- 原厂 ISP 参数标注为 1440p60，所有模式复用同一套参数；头文件按字节原样保留。
- 原厂参数与 SDK 的 AWB 字段兼容性告警、偶发 I²C 重试需按日志如实记录；短时出图不代表这些告警均已解决。
- 4K 两次测试均在停流边界出现一次 `tdm0 rx0 frame error`，持续收帧统计窗口内没有帧错误。此停流告警仍保留，不能将本版描述为所有阶段零报错。
- 本仓库不宣称完成各模式的长期零丢帧编码、全部场景画质验收或对不同硬件版本的兼容性认证。

先前 2K-only 版本的干净 SDK 全量复现记录见 [历史验证](docs/clean-sdk-with-sd-validation.md)，不能替代本轮新增模式验证。
