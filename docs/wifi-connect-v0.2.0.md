# Avaota F2 TF WiFi 联网操作（v0.2.0）

适用于本 Release 的 Tina V861 v1.1 / Avaota F2 NOR 固件和 RTL8189FS SDIO WiFi 卡。先校验 SHA256SUMS，再完整烧录本版固件。

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

本轮已验证 WPA2 认证、DHCP、网关/局域网/外网 IP 通信、DNS，外网 HTTP，以及双向 TCP 文件校验，详见 [WiFi 联网实测](https://github.com/dshanpi/avaota-f2-patches/blob/v0.2.0/docs/wifi-network-validation.md)。这不是无线吞吐率或长时间稳定性认证。

配置放在 `/tmp`，断电后清除；本版自动加载驱动，连接路由器仍按上述步骤手动执行。需要保留配置时可将自己的配置以 600 权限保存到 `/etc/wpa_supplicant.conf`，启动时改用该路径；这不会自动增加开机连接服务。修改网络前先结束已有进程：

```sh
[ ! -f /tmp/udhcpc-wlan0.pid ] || kill "$(cat /tmp/udhcpc-wlan0.pid)"
wpa_cli -p /tmp/wpa_supplicant -i wlan0 terminate
```

如果已经完整应用 `b01d6da` 的驱动版，可用 `patches/avaota-f2-wifi-network-from-b01d6da.patch` 的两文件增量后重新编译并烧录；原始 SDK 请按照源码 ZIP 内 README 覆盖全部 21 个 source 文件或应用完整补丁。


如需使用摄像头测试包原脚本，请断电换回普通 FAT32 TF 存储卡；SDIO WiFi 卡不会挂载为 /mnt/SDCARD。
