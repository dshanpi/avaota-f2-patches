# 干净 SDK 编译及上板复现

日期：2026-10-22。结论：仓库替换文件可在同版本原始 SDK 上完整编译、打包，烧录后原版采集程序可以获取 1440p30/60 图像，原厂 ISP 参数已生效。

本页保留 SD 修复合入前的 11 文件摄像头基线记录。当前仓库包含 13 个源文件，
当前 13 文件版本已再次从原始 SDK 完整编译并烧录，见 [摄像头与 TF 卡完整复现](clean-sdk-with-sd-validation.md)。
初次 SD 增量验证记录见 [SD 卡说明](sd-card.md)。

## 基线与环境

- 仓库源码基线：`356be7934afaeb0dee2e1886d980ddbf85b225e1`。
- 原始包：`tina-v861-v1.1.tar.xz`，8,975,484,968 字节。
- 原始包 SHA-256：`66d357e95a5038837ff2287cd9620dd5a6b4d3d48ca592754328523898525246`。
- 主机：Ubuntu 24.04.4 LTS，已安装 SDK 所需编译依赖；8 vCPU、约 6 GiB RAM。
- 硬件：Avaota F2 NOR / 128 MiB RAM、DshanPI_CAM-OV13850-V1.0、四 Lane 转接板。
- 全新解压后，`out`、`.buildconfig`、OpenWrt `.config`、`staging_dir`、`build_dir` 均不存在。
- 重新克隆仓库，未复用之前调试 SDK 的编译产物；只覆盖仓库的 11 个源文件。

## 构建与文件验证

1. `sha256sum -c SHA256SUMS` 全部通过。
2. 完整补丁在未修改 SDK 上 `git apply --check` 通过；对原文件副本应用补丁后，11 个文件与 `source/` 逐字节相同。
3. 实际构建采用 README 的 `cp -a "$PATCH_DIR/source/." "$SDK_DIR/"`，覆盖后 11 个文件散列一致。
4. 依次执行 README 的 `autoconfig`、`loadconfig`、复制 OpenWrt defconfig、`openwrt_rootfs defconfig`、`./build.sh`、`./build.sh pack`，全部成功。
5. 全量构建约 21 分 47 秒；打包约 7 秒。没有额外源码修改，也没有从旧 SDK 复制库、模块或程序。
6. 检查编译后的 DTB 中 13 个路由/工作模式属性、模块无自定义参数、rootfs 模块自动加载配置，均正确。
7. 原版 `sample_virvi.c` 未修改，SHA-256 为 `7b7c543bd64ea8d0b99ea16ef76d68385e66cea868d310db0223dee5618b13e9`。
8. 新程序确实链接 `ov13850_mipi_isp610_20261021_195907` 参数。LYNX 完整烧录、校验和重启均成功。

首次运行 SDK 需要交互阅读并确认声明，不能关闭 stdin 后无人值守等待。本次最初的非交互配置停在提示处，终止后在交互终端完成确认，再继续原命令序列。README 已补充这一点及 GitHub 认证说明。

## 原版示例上板结果

四轮均运行 35 秒，使用同一个新编译的原版 `sample_virvi`，没有替换热修复模块。
通过 ADB 传入程序和配置，保存到 `/tmp`，不依赖 TF 卡。统计开流约第 10～30 秒的 VI 计数增量，每个窗口约 20.02 秒。

| 配置 | VI 计数估算 fps | lost_cnt 增量 | error_cnt 增量 | 图像 |
|---|---:|---:|---:|---|
| 2K30 存图 | 30.020 | 268 | 0 | 完整单帧 |
| 2K60 存图 | 59.990 | 960 | 0 | 完整单帧 |
| 2K30 无存图 | 30.020 | 0 | 0 | 不存图 |
| 2K60 无存图 | 59.990 | 0 | 0 | 不存图 |

VI 计数包括未能交付应用的帧，不能将存图配置这一列的帧率解释为应用交付帧率。
存图程序的 `GetCSIFrameThread` 在首帧保存后仍调用 `copyVideoFrame`，保存线程还继续清零整帧缓冲；
关闭这一条路径后，两个无存图短测窗口的 `lost_cnt` 增量均为零。这里只增加测试配置和说明，不改 SDK 示例或公共 VIN/ISP 代码。

四轮 `vi0` 输入均为 2560×1440 BGGR10、输出为 2560×1440 NV21；MIPI 显示四 Lane、RAW10、`int_pending=0xf (ok)`。
30/60fps 日志及 ISP 调试信息均匹配对应的 OV13850 原厂配置。所有程序正常退出，报告 `test result: success`。

两张 NV21 均为 5,529,600 字节，板端与取回文件 MD5 一致，并转换为图片目视检查：完整天花板/窗框画面，无明显撕裂或行错位。
场景主要为白色天花板，不构成色卡或完整画质验收。出图文件 SHA-256：

- 2k30：`b4c6abd70cced5f5943cfa0917d231f2d5093ac9e966f18eb043dd37e5272749`。
- 2k60：`a72cb2ed5235a53ec213fc0ee07694d1ca3d5c08ef36680f5981a03e4ad104bd`。

## 本次构建产物

- `out/v861/kernel/staging/sunxi.dtb`
  - SHA-256：`67cb19d0191100cee9347c5c135957a22a81567c371c7b08320bbf746fb58ea0`；96,619 字节。
- `out/v861/kernel/staging/lib/modules/6.6.0/ov13850_mipi.ko`
  - SHA-256：`494abffa0fb524aea75a72a1d43460a30322c2d873eff6ad120f0e9ff75fb588`；15,352 字节。
- `platform/allwinner/eyesee-mpp/middleware/sun252iw1/sample/sample_virvi/sample_virvi`
  - SHA-256：`74e4a25ced1975c536b685f6ea6b9afe62212e74f4188abe754798f9e9ed9f97`；5,357,804 字节。
- `out/v861_linux_avaota_f2_uart0_nor.img`
  - SHA-256：`35403523abc9a50fc62bf832434392113137494de19e36b75bc60b2dcbc10d48`；14,897,152 字节。

固件未提交到本源码仓库。构建时间、路径等可能进入产物，重新构建不保证二进制散列相同；上述散列用于识别本次实测产物。

## 验证边界

- 无存图短测的计数零增量仅限上述窗口，不代表长时间或编码零丢帧。
- 原厂 AWB 字段整数截断告警、初始 AEWB 统计、TDM 事件订阅、sensor_temp 和停止回调告警仍存在，详见 [原有验证记录](validation.md)。
- 30fps 共用原厂 60fps ISP 参数，未做独立 IQ 标定。
- 本次没有验证 4K、编码或音视频长时间录制。
- Windows ADB 在当前机器显示在线但 shell 超时；本次通过 Ubuntu 虚拟机 ADB 完成传输。此环境问题不需要增加 Sensor 补丁。
