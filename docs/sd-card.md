# TF 卡自动识别与挂载

适用：Tina V861 v1.1、Avaota F2 NOR。测试日期：2026-10-22。
相对摄像头基线 `b61eff60bbb8222708bae91eaedcf9c2ea4a4463`，SD 修复只涉及四个文件。

后续已从原始 SDK 再次全新解压，使用新仓库的全部 13 个文件完整编译、烧录，
并重新验证 2K30/60 出图、TF 启动挂载和实际拔插后的自动挂载。
最新镜像及结果见 [摄像头与 TF 卡干净 SDK 完整复现](clean-sdk-with-sd-validation.md)。
下面保留初次 SD 增量验证记录，文中的镜像摘要属于该历史轮次。

## 修改范围

| SDK 相对路径 | 修改及原因 |
|---|---|
| `device/config/chips/v861/configs/avaota_f2/linux-6.6-xuantie/board.dts` | SDC0 删除 `cd-gpios`，增加 `broken-cd`；插卡状态曾与 PF6 检测值不一致，改用 MMC 标准轮询。 |
| `openwrt/target/v861/v861-avaota_f2/defconfig` | 原配置未启用 mdev；启用 mdev、规则配置、脚本执行和 netlink daemon。 |
| `openwrt/target/v861/v861-avaota_f2/busybox-init-base-files/etc/init.d/S05mdev` | 新增六行启动脚本，运行 `mdev -d` 和冷插拔扫描；内核未提供 uevent helper，使用 netlink 事件。 |
| `openwrt/package/allwinner/system/busybox-init-base-files/files/usr/bin/hotplug.sh` | 增加一行已挂载判断及注释，避免重复扫描报错。 |

没有设置 `non-removable`，仍支持插拔；没有修改 MMC 公共驱动、摄像头时序、VIN/ISP 公共代码或内存池。
其余九个摄像头源文件与基线逐字节相同，原厂 ISP 头文件也完全保留。
轮询是对当前卡检测问题的软件绕过，不能据此认定 PF6 电路故障原因。

## 使用

干净 SDK 使用根目录 README 的完整文件替换或完整补丁。
已应用 `b61eff6` 摄像头配置的 SDK 使用 `patches/avaota-f2-sd-poll-automount.patch`。
完整补丁 `patches/avaota-f2-ov13850-2k-sd.patch` 已包含 SD 部分，不要叠加应用。
应用后重新生成 OpenWrt 配置、编译、打包并烧录，命令见根目录 README。

SDK 原有 `/etc/mdev.conf` 和 `/usr/bin/hotplug.sh` 负责挂载。本次验证的分区和路径为：

```text
/dev/mmcblk0p1 /mnt/SDCARD vfat rw
```

检查：

```sh
cat /proc/partitions
grep mmc /proc/mounts
df -h /mnt/SDCARD
pidof mdev
```

拔卡前停止采集、录像及其他写入，再执行 `sync; umount /mnt/SDCARD`。
自动识别和卸载不能保证带写入拔卡的数据安全。

## 初次 SD 增量构建实测结果

测试卡为 SU08G、7.40 GiB SDHC、FAT32 单分区。未格式化卡。

| 项目 | 结果 |
|---|---|
| 固件构建与烧录 | `./build.sh`、`./build.sh pack` 成功；LYNX 写入及校验成功。 |
| 启动检测 | 约 1.5 秒识别卡和第一分区，自动挂载到 `/mnt/SDCARD`。 |
| 读写与持久化 | 新目录写入 1 MiB 随机文件，重启后读回 MD5 一致。 |
| 重复扫描 | `mdev -s` 不再触发重复挂载报错。 |
| 事件链路 | 注入 remove/add uevent 后自动卸载、重新挂载，文件校验一致。 |
| 实际拔插 | 同一次启动中记录到 card removed 和重新识别，自动挂载后校验一致。 |
| 提交前复查 | 板子仍自动挂载为读写，mdev 守护进程在线，原测试文件校验一致。 |

测试文件 MD5：`5d7dc0a3f97a685f3c353aa24fc3bfca`，长度 1,048,576 字节。
已烧录实测镜像 SHA-256：`d0c4470790084054ca1fadb6f1f5fb4e66b4ea06ba2f5e541c27f98ef3a85e4f`。
构建时间戳会影响镜像摘要，此值用于标识测试镜像，不是所有重新构建的预期摘要。

实际拔卡时，固件重启已将卡重新挂载，留下了 FAT 未正常卸载标记。
随后在卸载状态用临时 dosfstools 检查，只发现 dirty bit；清理后再次只读检查正常，
重新自动挂载后文件校验仍一致。该检查工具没有加入固件或补丁。

## 仓库一致性复核

- 完整补丁针对由已验证摄像头补丁反向恢复的原始文件副本检查并应用；
  SD 增量补丁针对 `b61eff6` 文件副本检查并应用，两者均与 `source/` 的 13 个文件逐字节一致。
- `source/` 与实际编译 SDK 的 13 个文件逐字节一致，启动脚本执行位保留。
- 原厂头文件的文件末尾无换行，完整补丁保留对应标记，未改写头文件内容。
- 提交前再次按 README 覆盖文件、生成 OpenWrt 配置并增量构建和打包。
  这次复核复用已完成全新 SDK 构建的工作目录，不宣称再次全新解压或无缓存全量构建。
- 构建后检查 13 个摄像头 DTB 路由属性、SD 轮询属性、mdev 和启动脚本均通过；
  原版采集程序源码未改动，生成程序包含原厂 ISP 参数。
- 本次重新打包镜像 SHA-256 为 `10c2b672627377c32ac3957da9f83796f1cf9ff07c02a8fef77fc79ac027a7dc`。
  此镜像未再次烧录，板端复查使用上面标识的已验证镜像；两者的 13 个适配源文件相同。

## 边界

- 本次只验证上述 FAT32 卡，未验证其他卡、exFAT/ext4/NTFS 或多分区自动挂载。
- 无卡时，轮询可能输出 MMC 探测超时；拔卡瞬间也会有命令超时，未修改驱动隐藏日志。
- SDK 继承的 card-pwr-gpios 解析及 UHS tuning 提示仍存在；当前 DDR50 模式完成了上述读写测试。
- 未额外进行最终固件反复断电冷启动、长时间 SD 压力测试或本轮摄像头长测。
  摄像头基线测试见 [干净 SDK 记录](clean-sdk-validation.md)。
