# Avaota F2 OV13850 v0.1.0：烧录和摄像头测试

适用：Avaota F2、Tina V861 v1.1 NOR、Linux 6.6 Xuantie、DshanPI_CAM-OV13850-V1.0，经已核对引脚的转接板连接四 Lane 摄像头接口。其他模组或转接板须先核对引脚。本版是采集出图功能验证版本，不是长期编码或完整 ISP 画质验收版本。

## 下载什么

从同一个 GitHub Release 下载以下文件：

| 文件 | 用途 |
|---|---|
| `v861_avaota_f2_ov13850_multimode_minimal_test_20261022.img` | 本次已烧录验证的完整 NOR 固件 |
| `ov13850-camera-test-v0.1.0.tar.gz` | 板端 RISC-V 32 位采集应用、14 份配置、一键脚本和本说明 |
| `avaota-f2-patches-v0.1.0.zip` | SDK 修改文件、完整/增量补丁、应用补丁、README 与测试记录 |
| `CAMERA_TEST.md` | 本说明，单独提供便于查看 |
| `SHA256SUMS` | 上述四个文件的 SHA-256 校验值 |

固件 SHA-256：`22678fd1d7384cfd4913cfdc15dfddf90fefdfe50a5869624966844a338d6e8b`。

在 Ubuntu 下载目录验证：

```bash
sha256sum -c SHA256SUMS
```

Windows PowerShell 可用：

```powershell
Get-FileHash -Algorithm SHA256 .\v861_avaota_f2_ov13850_multimode_minimal_test_20261022.img
```

## 1. 烧录和准备

1. 摄像头接四 Lane 接口；按已核对的方向接好排线。更改摄像头接线时先断电。
2. 用 LYNX 或兼容的全志烧录工具，将本 Release 的 `.img` 完整烧录到 Avaota F2 NOR，等待校验完成再重启。完整烧录会覆盖板上原固件及 NOR 用户分区，需要保留的数据先备份。
3. 插入 FAT32 TF 卡；本版会自动挂载到 `/mnt/SDCARD`。以下测试将程序、图像和日志存到 TF 卡，不写满 NOR 根文件系统。
4. USB 数据线连接测试主机。Windows ADB 若显示在线但命令超时，可将 Tina ADB USB 连接到 Ubuntu 虚拟机后执行以下命令。

固件会自动加载 Sensor 模块，不需要带参数 `insmod`。默认 Sensor 模式为 2K30，具体测试模式由应用在开流前选择。

## 2. 解压并通过 ADB 传输

以下命令在 Ubuntu 主机执行；采集二进制是给开发板使用的，不能在 x86 主机上运行。

```bash
tar -xzf ov13850-camera-test-v0.1.0.tar.gz
cd ov13850-camera-test
sha256sum -c SHA256SUMS
cd ..

adb devices -l
```

只有一块 ADB 设备时可以直接使用 `adb`。多块板或重复序列号时，按 `adb devices -l` 中的 `transport_id` 指定目标；每次重新插拔后重新确认，不要照抄示例中的编号：

```bash
# 例如目标摄像头板这一轮的 transport_id 是 7：
ADB=(adb -t 7)
# 若只有一个目标设备，则使用：ADB=(adb)

"${ADB[@]}" shell 'cat /proc/sys/kernel/random/boot_id; uname -a'
"${ADB[@]}" shell 'grep " /mnt/SDCARD " /proc/mounts; df -h /mnt/SDCARD; lsmod | grep ov13850'
"${ADB[@]}" push ov13850-camera-test /mnt/SDCARD/
"${ADB[@]}" shell 'chmod +x /mnt/SDCARD/ov13850-camera-test/run.sh /mnt/SDCARD/ov13850-camera-test/bin/*'
```

必须确认 TF 卡已经挂载后再传输。若没有挂载，不要仅靠 `mkdir /mnt/SDCARD` 继续运行。
如果板上已有同名测试包，先取回其中 `results/` 的历史结果，再更新程序。

Windows 主机也可用新版 `adb.exe` 执行对应命令。上面的 `ADB=(...)` 是 Bash 数组语法，PowerShell 中改为逐条执行 `& '完整路径\adb.exe' -t <实际编号> shell ...`。

## 3. 存一张图

先把镜头对准正常实物场景。以下以 1080p60 为例：

```bash
"${ADB[@]}" shell 'sh /mnt/SDCARD/ov13850-camera-test/run.sh 1080p60 save'
"${ADB[@]}" pull /mnt/SDCARD/ov13850-camera-test/results ./camera-results
```

脚本运行约 35 秒，输出结果目录、VI 帧率、丢帧/错误增量和退出状态。每轮新建目录，不覆盖之前的图片。可用模式：

| 模式参数 | 分辨率 | 目标帧率 | NV21 单帧字节数 |
|---|---|---:|---:|
| `720p30` / `720p60` | 1280×720 | 30 / 60 | 1,382,400 |
| `1080p30` / `1080p60` | 1920×1080 | 30 / 60 | 3,110,400 |
| `1440p30` / `1440p60` | 2560×1440 | 30 / 60 | 5,529,600 |
| `4k25` | 3840×2160 | 25 | 12,441,600 |

例如 4K 存图：

```bash
"${ADB[@]}" shell 'sh /mnt/SDCARD/ov13850-camera-test/run.sh 4k25 save'
```

结果目录中主要有：

- `frame.nv21`：原始 NV21 图像，仅 save 模式产生；文件不是 JPEG。
- `capture.log`、`exit-code.txt`：应用日志和退出码；正常为 0。
- `fps.txt`、`vi-t10.txt`、`vi-t30.txt`：实际统计结果和原始计数。
- `mipi.txt`、`isp.txt`、`dmesg-before.txt`、`dmesg-after.txt`：通路和内核日志。
- `frame.md5`：板端图片摘要，可与主机取回文件比较。

## 4. 查看图片

在主机使用支持 NV21 的 YUV 查看器，指定准确的分辨率。也可在已经安装 FFmpeg 的主机上转换，例如 1080p：

```bash
ffmpeg -f rawvideo -pixel_format nv21 -video_size 1920x1080 \
  -i frame.nv21 -vf 'scale=in_color_matrix=bt709:in_range=limited:out_range=full' \
  -frames:v 1 frame.png
```

4K 改为 `3840x2160`，2K 改为 `2560x1440`，720p 改为 `1280x720`。配置使用 REC709 limited range；采用错误色彩矩阵或把 NV21 当成 NV12 会影响显示颜色。

## 5. 测持续收帧和帧率

存图示例保存第一张后仍有整帧复制开销，尤其高分辨率时会产生明显丢帧，不能用 save 模式证明持续输出帧率。测试持续收帧使用 `stream`，它不保存图片：

```bash
"${ADB[@]}" shell 'sh /mnt/SDCARD/ov13850-camera-test/run.sh 4k25 stream'
"${ADB[@]}" shell 'sh /mnt/SDCARD/ov13850-camera-test/run.sh 1080p60 stream'
```

可以依次测试全部模式：

```bash
for mode in 720p30 720p60 1080p30 1080p60 1440p30 1440p60 4k25; do
  "${ADB[@]}" shell "sh /mnt/SDCARD/ov13850-camera-test/run.sh $mode stream" || break
done
"${ADB[@]}" pull /mnt/SDCARD/ov13850-camera-test/results ./camera-results
```

脚本在约第 10/30 秒读取 VI 计数，按实际 uptime 间隔计算帧率；低分辨率统计 `vi0`，4K 统计 `vi1`。例如：

```text
VI window: 20.030s, fps: 25.012, lost delta: 0, error delta: 0
```

这是 VI 硬件采集统计，不是应用交付、H.264/H.265 编码或显示帧率。请同时检查尺寸、图像、退出状态及完整日志，不能只凭一行 fps 判定所有问题已解决。

## 应用组成及自行编译

测试包内有两个实际用于本次验证的程序，脚本自动选择：

- `bin/sample_virvi-original`：原版 SDK 采集示例，用于 1080p 和 2K。
- `bin/sample_virvi-modes`：独立应用补丁版本，用于 720p 和 4K。720p 在 ISP 启动前设置 D3D LBC 比例 100；4K 请求两个 YUV 缓冲。

两者均链接此次原厂 ISP 参数，依赖匹配固件中的 `/usr/lib/eyesee-mpp` 等动态库。不要拿其他 SDK 的旧程序替换测试。ISP 参数静态链接进应用，只替换 Sensor `.ko` 不会更新 ISP。

应用源码基础是 Tina V861 v1.1 中的 `platform/allwinner/eyesee-mpp/middleware/sun252iw1/sample/sample_virvi/sample_virvi.c`。独立差异在包内 `source/sample_virvi-multimode.patch` 和仓库 `test/sample_virvi-multimode.patch`。按仓库 README 应用源码/配置后编译：

```bash
./build.sh openwrt_rootfs package/subpackage/allwinner/vision/libAWIspApi/compile V=s
./build.sh openwrt_rootfs package/subpackage/allwinner/eyesee-mpp/middleware/compile V=s
```

输出程序位于上面 `sample_virvi` 源码目录。固件源代码对应 `3589fd6bc704c0e5e7435b7e691d20f5f384ace7`；Release 后续整理提交仅添加发布说明和测试脚本，未改变固件源文件。

## 已验证结果与限制

七档均取得完整图像；每档各一次 35 秒存图测试和持续收帧测试，约 20 秒统计窗口内：

| 模式 | 持续收帧 VI fps | lost/error 增量 |
|---|---:|---:|
| 720p30 / 60 | 30.020 / 59.975 | 0 / 0 |
| 1080p30 / 60 | 30.012 / 59.975 | 0 / 0 |
| 1440p30 / 60 | 30.012 / 59.960 | 0 / 0 |
| 4K25 | 25.012 | 0 / 0 |

4K 使用 SDK 的单帧左右分块处理和合并，其他模式不拼接。**4K 停流边界仍会出现一次 TDM 帧错误告警**，正常收帧统计窗口内没有该错误；随后切回 2K60 复测正常。偶发 I²C 写重试、原厂 ISP 与 SDK 的参数兼容性告警仍保留，没有宣称全部报错已解决。

原厂 ISP 参数原样复用，各分辨率未独立完成画质标定；本版未完成长时间录制、编码性能和量产验收。更多原始数据见仓库 `docs/multimode-validation.md`。

测试结束拔卡前，先确认采集已退出，再执行：

```bash
"${ADB[@]}" shell 'sync; umount /mnt/SDCARD'
```
