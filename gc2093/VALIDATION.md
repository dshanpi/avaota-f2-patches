# GC2093 驱动验证，2026-11-10

板卡：V861 Avaota F2；配置 v861-avaota_f2-tina。LYNX project19/device22，COM117。模组图片标记内置24MHz晶振，7位 I²C 地址0x7e。

本次新增修改只有设备树 `sensor0_twi_addr` 从0x7e改为0xfc；其余 GC2093 驱动、内核选项、OpenWrt 模块选项已由用户 SDK 预先选好。参考快照保留这些配置，不将用户原有其他配置算作本次新增修改。

## 编译和烧录

- `./build.sh kernel` 成功，`./build.sh pack` 成功。复用用户 SDK 已有 rootfs，未做全新 SDK 完整复编。
- 固件：v861_avaota_f2_gc2093_1080p30_test_20261110.img，14897152字节。
- SHA256：e354ae160b5b5f862e8442a0a8bd7df75a9eb390c978218bdcd3aa7bb68848a4。
- LYNX flash-1794299305377933000：state=success，progressPct=100，verifyState=success，postFlashState=success，writtenBytes=10463324。存储类型 SPI NOR 16MB。写入中观测速率约0.8MB/s，USB速率字段unknown；未将传输字节数视作独立的介质读回哈希。
- 实际启动：Linux 6.6.0 #2 SMP Tue Nov10 03:27:07 EST2026。LYNX post-flash曾重试一次设置模式，最终状态成功，并已独立验证 Linux 启动。

## 自动识别

启动日志直接读出 ID_VAL_HIGH=0x20、ID_VAL_LOW=0x93。生成 /dev/video0、4、8、12。无需在 insmod 中额外传地址参数。

## RAW收帧

测试程序gc2093_raw_test，1920×1080，RAW10，V4L2 fourcc BG10，单平面。GC2093驱动报告输入 Bayer 为SRGGB10。用户态请求的fourcc不等价于已经验证了Bayer重排；预览按驱动报告的RGGB解释。

每帧4177920字节，即1920×1088×2，包含高度对齐填充。驱动G_FMT返回stride/sizeimage为0，本程序使用QUERYBUF长度及DQBUF bytesused保存原始帧。

| 测试 | 帧数 | 时间戳跨度 | 实测fps | ERROR标志 | 帧序号缺口 | 程序返回 |
|---|---:|---:|---:|---:|---:|---:|
| 保存一帧短测 |300|9.997970s|29.9061|0|1|2|
| 不保存一分钟复测 |1800|59.954567s|30.0061|0|0|0|

程序按(N−1)/(最后帧时间戳−首帧时间戳)计算。短测出现1次帧序号跳变，未被忽略；后续一分钟没有复现。本结果不能代替长时间可靠性验证。

TF卡：/mnt/extsd/gc2093/frame.raw、raw-test.log、raw-long.log，并保留gc2093_raw_test程序。已sync，没有格式化TF卡。启动存在预先已有的FAT未正常卸载警告。

raw-preview.png是从已保存RAW中抽取96×54组RGGB 2×2像素、仅添加显示gamma并放大形成的快速预览。可辨认窗框和室内物体。存在偏色和高光饱和；没有白平衡、色彩校准、降噪和ISP调优，不作为最终图像质量样张。

## 当前边界

已验证：上电识别、自动加载、1080P30双Lane线性RAW收帧、保存、一次启停和再次采集。

未验证：GC2093专用ISP效果、YUV/编码、HDR、其他分辨率/帧率、全新SDK复编、长时间可靠性、断电冷启动。软件烧录重启已验证，LYNX当前没有电源控制配置。

日志中仍有SDK既有“get csi isp clk fail”警告和1080高度16对齐提示；本次采集未看到MIPI错误风暴。不能宣称系统日志零警告。
