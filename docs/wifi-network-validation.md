# RTL8189FS WiFi 联网验证（2026-10-23）

## 本次补充

在已验证的 TF 存储/WiFi 驱动版 `b01d6da` 上，增加联网所需的用户态构建配置，仅修改两个 SDK 文件：

1. `openwrt/target/v861/v861-avaota_f2/defconfig`：启用 BusyBox udhcpc、wpa_cli。
2. `openwrt/openwrt/package/network/services/hostapd/Makefile`：将本板原生 `kmod-tf-rtl8189fs` 纳入 nl80211 后端、libnl-tiny 依赖和链接及配置重建条件。

原固件的 wpa_supplicant 仅有 wired 后端，且没有 DHCP 客户端；wlan0 出现并不代表具备 WPA2 联网工具。保留原生 cfg80211/8189fs；本轮没有修改内核驱动、设备树、摄像头、分区表或启动脚本，没有安装 backports cfg80211。未预置路由器名称或密码。

仓库现在有 21 个完整 source 文件；相对 v0.1.0 的 WiFi 累计增量为 12 个文件、447 行补丁；相对 b01d6da 的联网补充为两个文件、59 行补丁。三个补丁均在各自正确的 SDK 文件基线上应用后，与 source 文件逐字节一致。

## 编译与烧录

在上一轮从原始压缩包解压并全量编译过的 `/home/ubuntu/avaota-f2-tfwifi-clean-20261022/tina-v861-v1.1` 中补充这两个文件，重新执行 OpenWrt defconfig、hostapd 包 clean、完整 `./build.sh` 和 `./build.sh pack`；本次复用了该干净来源 SDK 的已有构建缓存，没有声称再次从零解压或删除全部输出。

构建成功后确认 rootfs 包含 wpa_cli、udhcpc 和 libnl-tiny，wpa_supplicant 的 drivers 列表实际包含 nl80211。LYNX 对绑定板卡完成完整烧录及校验，任务 `flash-1792721496315599800` 的 state、verifyState、postFlashState 均为 success。测试仅使用串口，未使用 ADB。

固件：`v861_avaota_f2_tf_wifi_network_20261023.img`，15,732,736 字节。

SHA-256：`cda75d791af8e6123d225e6f0f4402eedc26795700847888c790c1bedd6df6fe`。

boot 为 5,963,776 字节，等于原分区容量；rootfs 为 4,980,736 字节，小于 5,767,168 字节，余量 786,432 字节。后续修改需重新核对容量。

板端内核：`Linux 6.6.0 #3 SMP Thu Oct 22 22:09:02 EDT 2026`。

启动 ID：`96fbb7d1-8ddf-4c45-a352-cace32ab39c5`。

## 实测结果

| 项目 | 结果 |
|---|---|
| 开机自动驱动 | cfg80211、8189fs 自动加载，wlan0 正常出现，无手工 insmod |
| 扫描 | 发现目标 2.4GHz 路由器，2412MHz；扫描信号约 −62dBm，连接时约 −58dBm |
| WPA2 认证 | nl80211，WPA2-PSK，pairwise/group CCMP，`wpa_state=COMPLETED` |
| DHCP | 返回 0，IPv4 `192.168.1.39/24`；网关、DNS `192.168.1.1`；租约最初 172800 秒 |
| 网关 ICMP | 首轮 10/10，追加 30/30；README 命令复核后 3/3，均为 0% 丢包 |
| 局域网电脑 ICMP | `192.168.1.38`，5/5，0% 丢包 |
| 外网 IP ICMP | `1.1.1.1`，5/5，0% 丢包 |
| DNS | 通过 DHCP 下发的 DNS 正常解析 example.com |
| 局域网 TCP 下载 | 电脑 → 板端 1,048,576 字节，MD5 `c35cc7d8d91728a0cb052831bc4ef372` |
| 局域网 TCP 上传 | 板端 → 电脑 1,048,576 字节，电脑逐字节比较一致，MD5 同上 |
| 外网 HTTP | example.com:80 返回 `HTTP/1.1 200 OK`，收到 888 字节响应 |
| README 连接步骤复核 | 结束原连接后，以 README 的 `wpa_supplicant -B ...` 和 `udhcpc ...` 重新连接，启动/认证/DHCP/网关连通均成功 |

TCP 采用电脑临时测试服务及板端 nc，合成固定数据，不传输用户文件。短样本和应用层校验通过不等于吞吐率标定或所有底层帧均无丢弃；接口 RX dropped 计数出现过增加，不能描述为所有协议层零丢包。

BusyBox ping 不接受小数间隔，追加测试使用默认整数秒间隔；nc 是精简实现，发起外网 HTTP 请求时通过保持 stdin 短暂打开取得完整响应，不能仅以 nc 返回 0 判断已收到 HTTP 数据。

## 发布前重启复核

发布前再次检查发现板子已重启，启动 ID 为 `7a627255-3fcd-4bf1-8099-fec6abe7b17f`，仍运行同一 #3 内核。cfg80211、8189fs、wlan0 已自动存在；/tmp 中的配置按预期被清除。按 README 重新设置连接配置并启动认证与 DHCP，仍为 WPA2-PSK/CCMP、COMPLETED，获取 `192.168.1.39`，网关 3/3 无丢包。该项验证的是重启后按文档手动连接，没有增加开机自动连接服务。

## 使用与边界

README 已给出完整手动 WPA2 配置、后台认证、DHCP、IP/路由/DNS 检查、ping 和停止连接的方法。普通存储卡挂载与换卡驱动验证沿用 [TF 卡验证](tf-wifi-validation.md)；本联网版没有改动其相关代码，也未再次进行物理存储卡插拔测试。

真实测试凭据仅保存在板端 `/tmp/wpa_supplicant.conf`（600 权限），断电清除，未写入源码、补丁、README 或 GitHub。测试结束保持板端 WiFi 已连接，DHCP 客户端后台续租。

本轮不包括 WPA3、企业认证、隐藏 SSID、断电自动连接路由器、休眠唤醒、吞吐基准或长时间压力测试；驱动自动加载与路由器连接是不同步骤。
