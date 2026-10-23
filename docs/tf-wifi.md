# Avaota F2 TF 插槽：存储卡与 RTL8189FS WiFi 卡

适用 Tina V861 v1.1、Avaota F2 NOR、Linux 6.6 Xuantie。参考 `TFCard_fit` 的 RTL8189FS 适配，复用 SDK 自带驱动。本次目标是普通 TF 卡自动识别/挂载，以及换成 RTL8189FS SDIO WiFi 卡后开机自动出现无线接口。

同一个插槽每次使用一种卡。WiFi 卡是 SDIO 外设，不是存储卡，不会挂载成 `/mnt/SDCARD`。换卡验证以断电更换、再上电为准。

## 修改范围

相对 `v0.1.0` 共 12 个 SDK 文件：

| 文件 | 用途 |
|---|---|
| `bsp/drivers/net/wireless/rtl8189fs/Makefile` | RISC-V Kbuild、cfg80211、内置固件/参数及体积配置 |
| `os_dep/linux/ioctl_cfg80211.c` | Linux 6.6 的密钥、漫游、信道通知和 BSS 接口 |
| `os_dep/linux/os_intfs.c` | NAPI 与 MAC 地址 API |
| `os_dep/linux/rtw_proc.c` | `pde_data` API |
| `os_dep/linux/wifi_regd.c` | 移除的监管标志兼容处理 |
| `os_dep/osdep_service.c` | 线程退出与随机数 API |
| `device/config/chips/v861/configs/avaota_f2/linux-6.6-xuantie/board.dts` | SDC0 开 SDIO、DAT1 中断、3.3V、25MHz；关闭 auto_print |
| 同目录 `bsp_defconfig` | BSP cfg80211、RTL8189FS 模块 |
| `openwrt/target/v861/v861-avaota_f2/modules.mk` | 将匹配内核的两个模块装入 rootfs |
| 同目录 `defconfig` | 选择 `kmod-tf-rtl8189fs`、wpa_cli、BusyBox udhcpc |
| `openwrt/openwrt/package/network/services/hostapd/Makefile` | 原生模块对应的 nl80211、libnl 构建条件 |
| 同目录 `busybox-init-base-files/etc/init.d/S06tfwifi` | 开机自动注册 cfg80211、RTL8189FS 驱动 |

驱动表格中省略的目录前缀均为 `bsp/drivers/net/wireless/rtl8189fs/`。P2P、MP 测试、外置 EFUSE/PHY 参数文件和调试日志的配置沿用参考包的精简选项。AP 能力未额外裁剪。

保留既有 `broken-cd` 轮询与 mdev 存储卡挂载。普通存储卡仍走 `/usr/bin/hotplug.sh`，不修改 MMC 公共驱动。关闭 auto_print 是针对本 SDK SDIO 枚举可能卡住控制台的板级配置处理，实际 UART0 为 PH9/PH10。

启动脚本在每次开机注册两个模块，内核仅向匹配的 SDIO WiFi 卡绑定驱动。普通 TF 卡启动时也会注册模块，但不会产生虚假的 wlan0。模块注册与联网配置是两个步骤。

## 安装与编译

推荐从原始 Tina V861 v1.1 SDK 开始，按照仓库 README 复制全部 `source/` 文件并完整编译。若已完整应用 `v0.1.0`，可改用增量补丁：

```bash
cd "$SDK_DIR"
git apply --check "$PATCH_DIR/patches/avaota-f2-tf-wifi-from-v0.1.0.patch"
git apply "$PATCH_DIR/patches/avaota-f2-tf-wifi-from-v0.1.0.patch"
```

增量补丁和全部文件覆盖二选一。原始 SDK 则使用完整文件覆盖或完整补丁。

```bash
cd "$SDK_DIR"
./build.sh autoconfig -i v861 -o openwrt -b avaota_f2 -n nor -k linux-6.6-xuantie -a riscv32
./build.sh loadconfig
cp openwrt/target/v861/v861-avaota_f2/defconfig openwrt/openwrt/.config
./build.sh openwrt_rootfs defconfig
./build.sh
./build.sh pack
```

烧录新完整镜像 `out/v861_linux_avaota_f2_uart0_nor.img`。不能只复制 `.ko`，因为 SDIO 设备树、模块打包、启动脚本也需要生效。经 Windows 手动复制源码时确认 S05mdev 和 S06tfwifi 保留可执行权限。

## 使用存储卡

插入 FAT32 存储卡上电，板端串口执行：

```sh
cat /proc/partitions
grep mmc /proc/mounts
df -h /mnt/SDCARD
```

正常应看到 `/dev/mmcblk0p1` 挂载至 `/mnt/SDCARD`。安全取卡前先停止写入，执行 `sync; umount /mnt/SDCARD`。不要格式化或覆盖已有数据来验证。

## 使用 TF 接口 WiFi 卡

先断电，换成 RTL8189FS TF 接口 WiFi 卡，再上电。无需手工 insmod：

```sh
uname -r
lsmod
ls /sys/bus/sdio/devices
cat /sys/bus/sdio/devices/*/vendor
cat /sys/bus/sdio/devices/*/device
ls /sys/class/net
cat /proc/net/wireless
ifconfig wlan0 up
ifconfig wlan0
```

参考实物的 SDIO ID 是 `024c:f179`；接口预期为 `wlan0`。接口未启用时，直接 `ifconfig` 可能不显示 wlan0，可使用 `ifconfig -a` 或 `ls /sys/class/net` 查看；`ifconfig wlan0 up` 用于启用接口。出现 SDIO 设备却无 wlan0 时，检查 `dmesg`、`/lib/modules/$(uname -r)/cfg80211.ko`、`8189fs.ko` 及 S06tfwifi。其他芯片即使使用同样 TF 插槽，也不能直接套用此驱动。

当前联网版已补齐 nl80211 认证、wpa_cli 和 DHCP 客户端，手动连接步骤见仓库 README，实际认证/IP/通信结果见 `wifi-network-validation.md`。没有预置 SSID/密码或自动联网服务；仅看到 wlan0 不能证明已联网。

## 验证记录

实际干净 SDK 构建、烧录、WiFi 冷启动及普通存储卡换卡结果记录在 `tf-wifi-validation.md`。后续测试完成后以该文件为准。
