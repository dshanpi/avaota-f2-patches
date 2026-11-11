## Avaota F2 OV13850 RTSP 测试包 v0.3.0

发布 OV13850 RTSP 的源码、可执行应用、三档参数、配套固件和测试说明。**本轮确认 1080P30、2K30 能按文档出流和解码；4K 是未通过运行验收的实验模式。**

### 验证结果

从 GitHub 全新克隆，按 README 在现有 Tina V861 v1.1 SDK 执行 ISP、MPP 和 RTSP 运行包编译；通过串口解包、校验、逐档启停，经 WiFi RTSP/TCP 各录制约一分钟：

| 模式 | 实际解码帧率 | 本轮结论 |
|---|---:|---|
| 1080P30 | 30.007fps | 功能复测通过 |
| 2K30 | 30.010fps | 功能复测通过，建议演示使用 |
| 4K 目标 25fps | 15.883fps | **未通过**：持续在线编码 overwrite，最大帧间隔约 3.902 秒 |

三档交付给解码器的帧均能稳态解码，但 4K 在编码前丢帧/停顿，不能用“程序返回 success”或“视频能解码”作为通过依据。Windows VLC 另完成 2K 的 RTSP/TCP 软件解码检查；使用无界面输出，仍有启动等待参数集、迟帧提示，不是长期流畅性或画质标定验收。

### 附件

1. `v861_avaota_f2_tf_wifi_network_20261023.img`：配套 NOR 完整镜像，**沿用 v0.2.0 原镜像，没有重新编译固件**。已核对镜像 boot、rootfs 和 OV13850/WiFi/cfg80211 模块与本轮实测板一致。
2. `avaota-f2-patches-v0.3.0.zip`：本次 tag 对应的完整仓库快照，包含 21 个 SDK 覆盖文件、补丁、RTSP 新增源码、参数和文档。
3. `ov13850-rtsp-source-v0.3.0.tar.gz`：完整应用/测试源码包，含原版 `sample_smartIPC_demo` C/H、公共示例辅助代码、配置解析器、构建 makefile，以及 RTSP 修正库、ISP 导出器、三档配置和启停脚本。
4. `ov13850-rtsp-app-v0.3.0.tar`：可执行运行包，含 RISC-V 32 位原应用、修正 `.so`、25/30fps ISP bin、配置与脚本。**板端只传这一份 tar，不需要先编译。**
5. `ov13850-camera-test-v0.1.0.tar.gz`：沿用旧版普通 TF 卡采集测试工具，作为补充；本轮不重新宣称其全部 30/60fps 模式通过，方法见源码快照的 `docs/camera-test-v0.1.0.md`。
6. `RTSP_TEST.md`：预编译应用的串口运行方法、VLC 观看、PC 录像、完整源码编译方法和限制。
7. `WIFI_CONNECT.md`：WPA2 配网、DHCP 和网络检查方法，不含真实凭据。
8. `VALIDATION.json`：30 秒历史短测和本轮一分钟复现记录，含 4K 失败结论。
9. `FIRMWARE_INFO.json`：固件来源、分区/模块哈希、源码与应用对应关系。
10. `SHA256SUMS`：附件 SHA-256 清单，下载后先校验。

### 最快运行

配套固件启动、OV13850 接好、wlan0 已配网后，在 PC 传输：

```sh
adb push ov13850-rtsp-app-v0.3.0.tar /tmp/ov13850-runtime.tar
```

然后在开发板串口运行：

```sh
mkdir -p /tmp/ov13850-display
tar -xf /tmp/ov13850-runtime.tar -C /tmp/ov13850-display
rm /tmp/ov13850-runtime.tar
sh /tmp/ov13850-display/run.sh 1440p30
```

VLC 以 RTSP/TCP 打开 `rtsp://开发板wlan0的IP:8554/ch0`。切换前 Ctrl+C 并等待退出；1080P 用 `1080p30`。完整步骤见附件 `RTSP_TEST.md`。同一插槽不能同时插 SDIO WiFi 卡和普通存储卡；RTSP 包关闭板端录像，在 PC 保存视频。

### 源码与修改范围

新增 RTSP 配置、AE 参数导出、仅作用于应用进程的 `LD_PRELOAD` 修正，以及构建/运行脚本；不修改 SDK 原版 `sample_smartIPC_demo` C/H、Sensor、设备树、VIN/TDM/ISP 内核公共代码或系统库。源码包依赖相同版本 Tina SDK 的交叉编译器和 MPP 库，不包含完整 SDK 或厂商闭源库。

源码包重新构建生成的应用、修正库、ISP bin、配置和启停脚本，与本轮上板文件逐字节一致。实拍图片、视频、无线密码和本地认证信息未上传。

### 已知限制

- **4K 未通过稳定运行验收，不能作为稳定满 25fps 或可发布的 4K 编码承诺。**降低请求帧率未消除 overwrite，压缩格式双缓冲试验被当前 MPP 拼接路径拒绝；诊断试验未采用。
- 保留 4K 停流 TDM 告警、可选 sensor ioctl 和部分编码接口提示。RTSP H.265 连接初期仍可能等待参数集，通常下一个 I 帧后恢复。
- 一分钟功能测试不等于长时间稳定性、运动压力、低照度或完整 ISP 标定。推荐先展示 1080P30、2K30。
- 镜像沿用已验证的 v0.2.0：TF 存储卡自动挂载、RTL8189FS 驱动自动加载；路由器连接仍需按说明手动配置，不预置真实 WiFi 密码。

固件 SHA-256：`cda75d791af8e6123d225e6f0f4402eedc26795700847888c790c1bedd6df6fe`。
