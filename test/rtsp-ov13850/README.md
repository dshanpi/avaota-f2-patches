# OV13850 的 sample_smartIPC_demo RTSP 展示优化

此目录是 **RTSP 应用层的可选优化**，不属于摄像头驱动适配补丁。基于 Tina V861 v1.1 / Avaota F2 / OV13850 四 Lane，提供 1080P30、2K30 和 4K 目标 25fps 的 H.265 单路展示配置。

不修改 SDK Sensor、设备树、VIN/TDM/ISP 内核代码或系统库，也不修改 SDK 自带的 `sample_smartIPC_demo` 源码。新增内容是三档配置、ISP 参数导出源码、仅作用于应用进程的 RTSP 修正库，以及编译和启停脚本。**需要完整使用运行包，不是只复制一个 conf。**

## 文件与修改范围

| 文件 | 用途 |
|---|---|
| `profiles/1080p30.conf` | 1920×1080、30fps、H.265 VBR 6,291,456 bit/s |
| `profiles/1440p30.conf` | 2560×1440、30fps、H.265 VBR 10,485,760 bit/s |
| `profiles/4k25.conf` | 3840×2160、目标 25fps、H.265 VBR 12,582,912 bit/s |
| `src/export-isp.c` | 从 SDK 的原厂 OV13850 ISP610 参数导出 30/25fps 的运行时 bin，仅调整 AE 表 |
| `src/rtsp-tcp-writeall.c` | RTSP TCP 短写处理和最大 NAL 缓冲设置 |
| `build.py` | 编译新增源码，复制 SDK 原应用，生成可上板的无压缩 tar |
| `run.sh` / `stop.sh` | 按模式加载参数、启停应用、备份恢复已有全局 ISP bin |
| `validation.json` | 2026-11-11 短测结果及限制，不含实拍画面和网络凭据 |

三档使用初始 QP 28、I 帧 QP 20～34、P 帧 QP 22～36，GOP 约一秒；`stream_buf_size=-1` 跟随编码器 VBV，避免高质量大 I 帧超过原应用默认缓冲后被丢弃。音频、额外码流、OSD 和板端录像关闭。VBR 实际码率随场景变化，不会一直等于目标码率。

原厂 ISP 的 AWB、CCM、Gamma、降噪和锐化开关保持不变；预览、拍照、视频 AE 表统一整理为两段，分别按 30/25fps 导出并限制增益。没有启用实验性强锐化，也没有完成色卡、暗光和镜头阴影全套标定。

4K 使用当前固件已有的 ISP1/VIPP1 单帧分块合并路径，采用在线编码、节省内存的参考帧/ISP 配置。这里没有关闭或改写内核拼接实现；1080P/2K 为 ISP0/VIPP0 非拼接路径。

## 适用前提

- 按[仓库主 README](../../README.md)将本分支 `source/` 应用到相同版本的 SDK，编译并烧录支持 OV13850 全模式的固件。仅 GC2093 或仅 2K 模式的驱动固件不能直接使用全部三档。
- 验证板为 128MB DRAM，板级视频 size pool 约 50MiB；不要将本配置直接套到不同内存布局的固件。
- TF 插槽若插 SDIO WiFi 卡，就不能同时保存视频到普通 TF 存储卡；本包关闭板端录像，建议在 PC 上接收 RTSP 录制。
- 默认使用 `wlan0`，先按[WiFi 说明](../../docs/tf-wifi.md)连接路由器并获取 IP。包内没有保存 SSID 或密码。
- Ubuntu x86_64 主机需要 SDK 原有编译环境，以及 `python3`、本机 `gcc`。板端使用 SDK 的 RISC-V 32 位 musl 程序和库。

本次实测内核为 `6.6.0 #3 SMP Thu Oct 22 22:09:02 EDT 2026 riscv32`。其他应用二进制、固件或 ISP 布局需要重新验证。

## 1. 编译原版 SDK 应用

示例源码继续使用 SDK 中的原文件，没有在本目录再复制一份：

```text
platform/allwinner/eyesee-mpp/middleware/sun252iw1/sample/sample_smartIPC_demo/sample_smartIPC_demo.c
platform/allwinner/eyesee-mpp/middleware/sun252iw1/sample/sample_smartIPC_demo/sample_smartIPC_demo.h
platform/allwinner/eyesee-mpp/middleware/sun252iw1/sample/sample_smartIPC_demo/sample_smartIPC_demo_config.h
```

先完成主 README 的板级选择和固件编译。若已编译过 SDK，可以只重新编译 ISP 与 MPP：

```bash
SDK_DIR="$HOME/tina-v861-v1.1"
PATCH_DIR="$HOME/Downloads/avaota-f2-patches"
cd "$SDK_DIR"
./build.sh openwrt_rootfs package/subpackage/allwinner/vision/libAWIspApi/compile V=s
./build.sh openwrt_rootfs package/subpackage/allwinner/eyesee-mpp/middleware/compile V=s
ls -l platform/allwinner/eyesee-mpp/middleware/sun252iw1/sample/bin/sample_smartIPC_demo
```

不要用宿主机 gcc 直接编译整个 MPP 示例。它依赖 SDK 的交叉编译环境和 MPP 库。原应用直接运行时不会自动获得本目录的外部 ISP 参数和 RTSP 修正，需要使用下面的运行脚本。

## 2. 编译新增源码并生成运行包

在 Ubuntu 主机执行：

```bash
cd "$PATCH_DIR"
python3 test/rtsp-ov13850/build.py --sdk "$SDK_DIR"
cd test/rtsp-ov13850/build
sha256sum -c SHA256SUMS
```

输出：

```text
build/ov13850-display/
  app/sample_smartIPC_demo          SDK 编译的原应用
  lib/librtsp_tcp_writeall.so       本目录的 RTSP 修正库
  profiles/*.conf                  三档配置
  iq25/isp_param_config.bin        4K 参数
  iq30/isp_param_config.bin        1080P/2K 参数
  run.sh / stop.sh
  BUILD-INFO.json / SHA256SUMS
build/ov13850-runtime.tar           上板只传这个文件，约 7.4MB
build/SHA256SUMS                    tar 校验值
```

构建脚本不会写入 SDK：它在自己的 `build/.work` 中复制原厂参数头，只修正这份生成副本的相对 include，再用本机 gcc 导出 ISP bin。导出器要求 `ISP_VERSION=610`、参数结构长度 163984 字节；每个 bin 为 164058 字节。

编译原厂参数头时可能出现 AWB 数值转为 `unsigned char` 的警告；本目录保留原厂值，没有借此修改 AWB。生成文件与本次实际测试的 ISP bin 逐字节一致。

RTSP 修正库使用 SDK 的 `riscv32-unknown-linux-musl-gcc`，编译参数为：

```text
-shared -fPIC -Os -s -march=rv32imafdc -mabi=ilp32d -ldl -pthread
```

通常会自动找到 `prebuilt/rootfsbuilt/riscv/*/bin/riscv32-unknown-linux-musl-gcc`。若存在多个工具链，用 `--cross-cc /绝对路径/riscv32-unknown-linux-musl-gcc` 指定；可用 `--out /SDK之外的目录` 指定输出位置。

`BUILD-INFO.json` 记录原应用、原厂头文件的哈希和交叉编译器版本。如果应用哈希与实测样本不同，脚本会提示重新验证；这不代表新编译的应用一定不能使用。

## 3. 传输文件，统一通过串口运行

停止原先正在运行的摄像头应用。主机端 ADB 只用于文件传输，多块板时请加 `-s` 选择正确设备：

```bash
adb push "$PATCH_DIR/test/rtsp-ov13850/build/ov13850-runtime.tar" /tmp/
```

以下全部在**开发板串口**执行：

```sh
df -h /tmp
mkdir -p /tmp/ov13850-display
tar -xf /tmp/ov13850-runtime.tar -C /tmp/ov13850-display
rm /tmp/ov13850-runtime.tar
ifconfig wlan0
sh /tmp/ov13850-display/run.sh 1440p30
```

本板 `/tmp` 约 31MB，传输、解包前需预留约 15MB。先清理自己的旧录像或重复程序，不要把源码、测试视频等整个目录传入板端。BusyBox tar 不支持 `-z`，因此提供无压缩 tar 并使用 `tar -xf`。

PC 用 VLC 打开：

```text
rtsp://开发板wlan0的IP:8554/ch0
```

播放器选择 RTSP over TCP；可先设置 500～1000ms 网络缓存。建议只连接一个客户端。IP 由自己的路由器分配，不要照抄其他板子的地址。

在串口按 Ctrl+C，等待退出完成，再选择另一档：

```sh
sh /tmp/ov13850-display/run.sh 1080p30
sh /tmp/ov13850-display/run.sh 4k25
```

以上命令逐个运行，每次只运行一档。默认持续运行，也可限时：

```sh
DURATION=60 sh /tmp/ov13850-display/run.sh 1440p30
```

若在一个串口会话后台运行，可以从另一个串口终端执行：

```sh
sh /tmp/ov13850-display/stop.sh
```

脚本仅停止自己的子进程。退出后恢复原有 `/tmp/isp_test/color/isp_param_config.bin`；原本没有此文件时删除本次副本。SIGKILL 或意外终止不会执行清理；请先确认没有测试进程，再处理遗留的 `/tmp/ov13850-display.lock`。断电后 `/tmp` 内容清空，需重新传入运行包。本包不安装开机自启动服务。

## 4. USB 对照取流和 PC 录像

无线抖动明显时，可通过 USB 排除 WiFi 链路影响。在主机端建立转发：

```sh
adb forward tcp:18554 tcp:8554
```

在开发板串口运行：

```sh
NET_TYPE=0 sh /tmp/ov13850-display/run.sh 1440p30
```

主机播放 `rtsp://127.0.0.1:18554/ch0`；结束后主机执行 `adb forward --remove tcp:18554`。默认 `NET_TYPE=3` 为 WiFi。

也可以在装有 FFmpeg 的 PC 上录制原始 H.265 RTSP 流，不重复编码：

```sh
ffmpeg -rtsp_transport tcp -i rtsp://BOARD_IP:8554/ch0 -t 30 -an -c:v copy capture.mkv
```

实测使用下面的命令检查稳态解码，跳过连接最初 2 秒。明确保留帧时间和输出时基，避免默认输出时间基导致非递增 DTS 提示：

```sh
ffmpeg -hide_banner -v error -ss 2 -err_detect explode -i capture.mkv \
  -map 0:v:0 -fps_mode passthrough -enc_time_base 1:1000 -f null -
```

帧率按实际解码帧数和首末时间戳计算：`(帧数-1)/(最后一帧时间-第一帧时间)`。本 SDK 某些样本元数据会显示 50/120fps，不能直接当作实际帧率。

## RTSP 修正的原因与边界

SDK 的 `system/private/rtsp/liveMedia/RTPInterface.cpp` TCP 发送路径未处理 `writev()` 的短写。修正库只拦截本应用 RTSP 8554 端口的双 iovec、`$` 开头的 RTP/TCP 帧，补发剩余内容；超时约 5 秒或不可恢复的发送错误时关闭该连接，不继续发送截断字节流。其他 `writev()` 原样转发。它通过 `LD_PRELOAD` 只作用于应用子进程。

同一库通过应用导出的 `_ZN15OutPacketBuffer7maxSizeE` 符号，在启动时将最大 NAL 缓冲从 1,000,000 提高至 2,097,152 字节，不使用写死的内存地址。启动日志应有 `RTSP_NAL_BUFFER ... current=2097152`。如果没有该日志，需要检查应用是否导出此符号，不能直接假定修正已生效。

库退出打印 `RTSP_TCP_WRITEALL packets=... short_writes=... would_block=... failures=...`。短写/等待计数说明无线或客户端产生发送压力；客户端主动断开时也可能记录发送失败，需结合时间和视频判断。修正库不能提高无线实际吞吐量，也不保证零排队。

## 验证结果及未解决问题

2026-11-11，在同一块摄像头板上各录制约 30 秒；没有重烧固件，原应用未修改。新构建脚本输出另外做了字节一致性验证，见 `validation.json`。

| 模式 | USB 解码实测 | WiFi 解码实测 |
|---|---:|---:|
| 1080P30 | 30.009fps | 30.010fps |
| 2K30 | 30.013fps | 30.014fps |
| 4K 目标 25 | 24.030fps | 23.947fps |

三档以及最终 2K 启动脚本复测均通过上述稳态解码检查。脚本的退出清理、原有 ISP bin 备份恢复也已验证。推荐展示先用 **2K30**。

**4K 尚未达到稳定满 25fps**；4K WiFi 样本存在 3 处约 80ms 帧间隔。WiFi 仍可出现排队和抖动，4K 停流时仍能见到一次 TDM frame error；sensor flip/temperature 等可选 ioctl 以及部分编码接口提示尚未消除。SDK H.265 SDP 参数集顺序问题仍可能在连接初期产生 PPS/NAL 提示，等到下一个 I 帧恢复，本包未改其源文件。

早期较高内存配置曾触发分配失败和内核异常，重新上电后采用当前配置完成短测。本目录不是这些内核缺陷的修复补丁，也不代表长时间稳定性、满帧率或专业 ISP 标定通过。实拍场景在各轮之间有变化，不提供严格同场景画质改善幅度结论；没有将实拍视频、图片或 WiFi 凭据提交到仓库。
