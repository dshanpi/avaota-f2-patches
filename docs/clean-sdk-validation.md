# GC2093 干净 SDK 验证（2026-11-10）

## 基线及构建

- 原始包：`tina-v861-v1.1.tar.xz`，SHA-256 `66d357e95a5038837ff2287cd9620dd5a6b4d3d48ca592754328523898525246`。
- 生产修改对应提交：`c18bc09ad99d63129329ef728c9b8371f5c8c0e8`；之后仅完善测试和文档，五个 source 文件未改变。
- 从原始包新解压，不使用原主 SDK 的内核对象、rootfs、模块或 ISP 库。
- 按根 README 执行 autoconfig、覆盖 source、loadconfig、OpenWrt defconfig、完整 `./build.sh` 及 `./build.sh pack`，全部返回 0。
- 五文件补丁在原始文件上应用后与 source 逐字节一致；完整编译后再次核对五个 SDK 文件的 SHA-256 全部一致。
- 首次配置发现原 SDK 没有 GC2093 OpenWrt 包定义和 BusyBox 启动加载项，补齐后才开始完整构建。不能仅覆盖设备树或只添加内核选项。

## 固件及串口识别

- 固件：`v861_avaota_f2_gc2093_clean_20261110.img`，14899200 字节。
- 固件 SHA-256：`90531127d3b9af00c8ef78aed13551d090ace5a9645f1595d4fffe6ea7686208`。
- LYNX 完整擦写、校验、重启任务均成功。随后手动重新上电，串口恢复；本次 LYNX 未配置电源控制。
- 内核：`Linux (none) 6.6.0 #1 SMP Tue Nov 10 04:57:16 EST 2026 riscv32 GNU/Linux`。
- 启动 ID：`c2f652c8-4c87-4cb1-bcb5-51887c9a5977`。
- GC2093 自动加载，ID 高字节 0x20、低字节 0x93；`/dev/video0` 存在，无附加模块参数。
- 板端模块 MD5 `ae94cd588b5712b31671a24406db195a`，与构建目录中最终 rootfs 的已剥离模块一致。
- 内核 `CONFIG_SENSOR_GC2093_MIPI=m`，`CONFIG_SUPPORT_ISP_TDM` 未启用。

## 1080P30 实测

通过串口发送测试文件到 `/tmp`，应用 MD5 `4565f338d7753c49e192bd5345d905c4`；与干净 SDK 工具链编译出的最终测试应用一致。

第一轮旧测试程序在第 11 帧同步保存文件：1800 帧，29.9894fps，ERROR 标志 0、序号缺口 1，返回 2。该结果保留在 `capture-initial.log`，未将其算作通过。

测试程序随后改为保留最后一个已出队缓冲，停流后再保存；不额外分配整帧内存，也不改变摄像头驱动。按 README 再运行 `sh run-camera-test.sh`：

```text
RESULT frames=1800 elapsed=59.954551 fps=30.0061 error_flags=0 sequence_gaps=0
Capture exit code: 0
```

- 协商格式：1920×1080、单平面 BG10；实际模组输出双 Lane RAW10。
- 帧文件：4177920 字节，MD5 `a3b5d4d979658b46433fc7cf5ab0fff9`。
- 帧率来自首末 V4L2 时间戳，并非仅取设置值 30。

原 SDK 的 TF 检测在本次实物上没有识别到卡，故采用 README 的纯串口 /tmp 路径。TF 自动挂载、WiFi、ISP 调校、YUV、编码及长时间可靠性不属于本次验证范围。本记录不包含旧摄像头验证结果。
