# TF WiFi 干净 SDK 验证（2026-10-22～23）

本页保留驱动与换卡阶段的历史固件记录。后续认证/DHCP/网络通信及新版固件见 [WiFi 联网验证](wifi-network-validation.md)。

本轮源代码来自远端 `codex/tf-sdio-wifi` 分支提交 `9fc7f6c06049c1cf81020d9f60e505f09fa718ed`，在 `/home/ubuntu/avaota-f2-tfwifi-clean-20261022/tina-v861-v1.1` 完成。

## 已完成

- 从原始 SDK 压缩包重新解压；SHA-256 `66d357e95a5038837ff2287cd9620dd5a6b4d3d48ca592754328523898525246`，未复用旧工作目录的编译产物。
- 从 GitHub 重新克隆候选提交，复制 20 个 source 文件，依次执行 README 的 autoconfig、loadconfig、OpenWrt defconfig、完整 `./build.sh`、`./build.sh pack`。完整构建约 15 分钟，全部成功。
- 完整补丁与 v0.1.0 增量补丁分别在实际原始 SDK/已适配 SDK 的文件基线上还原后逐字节对比通过。新增到仓库的六个驱动完整文件，在 SDK 中本来存在，增量补丁只包含其修改 hunks。
- SDK 将 OpenWrt 新模块选项从 defconfig 尾部移到对应分类内，全部配置值一致；其余源文件校验一致。
- 编译后的 DTB 确认 SDIO、DAT1 中断、25MHz、3.3V、broken-cd 和 auto_print disabled；模块、执行权限、内核版本及目标 SDIO alias 均已检查。
- LYNX 完整烧录和校验成功。后端重启尾日志有“device not found”，以随后受控断电上电和串口实际新内核启动作为重启证据；烧录后需重新打开一次串口读句柄。
- 新内核 `Linux 6.6.0 #1 SMP Thu Oct 22 07:37:21 EDT 2026`，冷启动 ID `421c88c4-6ca2-424e-864c-43fd95edab72`。未手工 insmod，已出现 `8189fs` / `cfg80211`、SDIO `024c:f179`、绑定 `rtl8189fs`，以及 `wlan0`。
- `ifconfig wlan0 up` 返回 0，接口标志 `0x1003`（UP）；未配置 WiFi 关联或获取 IP，不把节点出现描述为已验证无线联网。

## 固件与容量

固件 `v861_avaota_f2_ov13850_tf_wifi_minimal_20261022.img`，15,601,664 字节。

SHA-256：`0bdde8321ca0062ac252b1e568010da70f299b6401ecddda4f86e8e44de46d55`。

原分区表保持不变：boot 镜像 5,963,776 字节，刚好等于该分区容量；rootfs 镜像 4,849,664 字节，分区 5,767,168 字节，余量 917,504 字节。后续改变内核配置时必须重新核对 boot 容量，不能沿用本次容量结论。

cfg80211 与 8189fs 的 vermagic 都是 `6.6.0 SMP mod_unload riscv`。没有安装另一套 OpenWrt backports cfg80211。

## 普通 TF 卡实测（2026-10-23）

用户将同一插槽换成普通 TF 卡后上电，串口确认仍是上述新内核：

| 项目 | 实测结果 |
|---|---|
| 初次换卡启动 ID | `8853781a-fbd2-49d5-9e26-ba2fae26c98e` |
| 存储卡 | SU08G，7.40 GiB，`mmcblk0p1` |
| 自动挂载 | `/dev/mmcblk0p1` → `/mnt/SDCARD`，vfat、可读写；未手工 mount |
| 安全独立读写 | 新建唯一 64 KiB 文件，大小 65,536 字节，写入、sync、读取成功；未覆盖原有文件 |
| 文件 MD5 | `49d2f6fe736d35a2eec693c849f76e4f` |
| LYNX 断电再上电 | 安全卸载返回 0 后，断电 3 秒、再上电成功 |
| 重启启动 ID | `e23fe402-a398-4583-8443-0a27d7b3f304` |
| 重启后自动挂载与数据保持 | 无手工 mount，挂载成功；同一文件大小、MD5 均一致 |
| 存储卡启动时网络节点 | eth0、lo、sit0，没有 wlan0；未将普通存储卡误识别为无线卡 |

测试文件为 `/mnt/SDCARD/tfwifi-check-8853781a.bin`，保留供复核。用户已有 FAT 卷在首次测试前即有“未正常卸载”的提示，安全卸载后仍可见该既有标志；没有自动格式化或修复文件系统。该提示不等于本轮读写校验失败，但本轮也不宣称完成整卡文件系统检查。

普通存储卡枚举时会尝试 SDIO CMD52/CMD5，出现 RTO 日志后继续识别为 SDHC 并正常挂载；应区分枚举探测日志与实际数据读写失败。本轮小文件校验未观察到数据错误，不代表所有卡型或长时间读写均通过。

## 验收范围

已完成用户要求的核心流程：远端仓库取源、干净 SDK 完整编译打包、LYNX 烧录校验、WiFi 卡冷启动自动加载驱动并出现可启用的 wlan0、普通 TF 卡冷启动自动挂载与读写、断电重启后自动挂载及文件校验。

普通卡热插拔追加测试已完成：安全卸载返回 0 后，用户保持板子上电拔卡、等待后重新插入。串口记录 `mmc0: card aaaa removed`，随后重新识别 SU08G 和 mmcblk0p1；未手工 mount，已自动挂载至 `/mnt/SDCARD`，vfat、可读写。启动 ID 仍为 `e23fe402-a398-4583-8443-0a27d7b3f304`，证明期间没有重新启动。文件仍为 65,536 字节，MD5 仍为 `49d2f6fe736d35a2eec693c849f76e4f`。未增加任何 SDK 修改。

拔卡及无卡轮询阶段可见 CMD13/52/8/5/55 超时，随后正常识别并挂载；本轮没有以“全部阶段无日志告警”作为结论。用户随后断电换回 RTL8189FS WiFi 卡并重新上电，第二轮冷启动复核也通过。启动 ID 为 `37254a6e-e0e8-4e0d-8cda-392b4258917a`，仍为同一新内核；未手工加载模块，`cfg80211`、`8189fs` 已存在。SDIO `mmc0:0001:1` 的 vendor/device 为 `024c:f179`，绑定 `rtl8189fs`；`/sys/class/net` 包含 wlan0。`ifconfig wlan0 up` 返回 0，接口标志 `0x1003`，即 UP/BROADCAST/MULTICAST。普通存储卡 → WiFi 卡的断电换卡流程已实际复核。

驱动创建接口后默认可以处于 DOWN，直接 `ifconfig` 只显示已启用接口，需用 `ifconfig -a` 或 `ls /sys/class/net` 检查节点。本轮没有新增自动启用、连接路由器或 DHCP 服务；手动启用成功不等同于已联网。

本轮按用户要求未使用 ADB。没有配置 SSID/密码，未验收无线关联、DHCP 或吞吐量；没有重复进行摄像头画质/帧率测试，摄像头源文件与既有版本一致。
