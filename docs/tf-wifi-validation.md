# TF WiFi 干净 SDK 验证（2026-10-22）

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

## 待完成

普通 FAT32 TF 卡的换卡自动挂载、读写和掉电保持，以及换回 WiFi 的冷启动复核尚待实际完成。当前 LYNX 的 Linux Workbench 状态读取持续持有硬件锁，电源操作被其 guard 拒绝；已请求结束该读取，不绕过硬件保护。用户已明确暂不使用 ADB，后续验证通过串口和电源控制完成。
