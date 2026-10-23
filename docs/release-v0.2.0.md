## Avaota F2 TF WiFi 联网版 v0.2.0

在既有 OV13850 多分辨率与 TF 自动挂载配置上，增加 RTL8189FS SDIO WiFi 驱动与 WPA2/DHCP 联网支持。适用于 Tina V861 v1.1、Avaota F2 NOR、Linux 6.6 Xuantie。

### 实测

- 普通 FAT32 TF 卡：自动挂载、64KiB 独立文件校验、断电重启后的文件校验、拔插后的自动挂载通过。
- WiFi 卡：重启自动加载 cfg80211/8189fs，出现 wlan0；断电换卡复核通过。
- 2.4GHz WPA2-PSK/CCMP 连接通过，DHCP 正常分配 IP、默认网关和 DNS。
- 网关、局域网电脑、外网 IP 的短样本 ping 均无丢包；DNS 与外网 HTTP 200 响应通过。
- 双向各 1MiB TCP 文件传输逐字节/MD5 校验一致；按 README 重连及发布前重启后的手动连接复核通过。

### 下载

1. `v861_avaota_f2_tf_wifi_network_20261023.img`：完整固件，LYNX 烧录并校验通过。
2. `avaota-f2-patches-v0.2.0.zip`：21 个完整 SDK 文件、完整补丁、各版本增量、README 与验证记录。
3. `avaota-f2-wifi-network-from-b01d6da.patch`：仅适用于已完整应用 b01d6da 的两文件联网增量；应用后重新编译烧录。
4. `WIFI_CONNECT.md`：从配置 SSID/密码、认证、DHCP 到网络检查的完整步骤。
5. `VALIDATION.md`：本轮实际联网测试与限制。
6. `ov13850-camera-test-v0.1.0.tar.gz`、`CAMERA_TEST.md`：沿用既有摄像头测试应用和方法；本轮未重新验收各模式画质。
7. `SHA256SUMS`：先校验全部附件，再烧录或使用。

WiFi 所需 wpa_supplicant、wpa_cli、udhcpc 已包含在固件内，无需另传认证程序。联网补充相对驱动版只改变板级 OpenWrt defconfig 和 hostapd 包 Makefile；不新增内核/摄像头/分区修改，不引入另一套 backports cfg80211。

### 使用边界

- 同一插槽每次使用一种卡。WiFi 卡是 SDIO 外设，不是存储卡；摄像头测试包默认脚本需要普通 TF 卡挂载到 /mnt/SDCARD，运行该脚本前断电换回存储卡。
- 驱动自动加载；连接路由器需按 WIFI_CONNECT 手动执行。示例配置位于 /tmp，断电清除，不预置任何真实网络凭据或开机自动连接服务。
- 本轮不宣称 WPA3/企业认证、隐藏网络、吞吐基准或长时间稳定性认证。接口 RX dropped 曾增加，应用层校验通过不等于所有底层数据帧均无丢弃。
- 既有 OV13850 模式代码保留，720p/1080p/1440p 30/60fps、4K25 的历史收帧记录见仓库；4K 停流阶段的既有 TDM 告警限制仍保留。

固件 SHA-256：`cda75d791af8e6123d225e6f0f4402eedc26795700847888c790c1bedd6df6fe`。
