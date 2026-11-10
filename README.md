# V861 Avaota F2 GC2093 驱动最小修改

`gc2093-support` 分支适配本次使用的 GC2093 模组：内置 24MHz 晶振、7 位 I²C 地址 0x7e、双 Lane 1920×1080 RAW10 / 30fps。连接开发板四 Lane 接口中的 MIPI0/CSI0。

已从原始 Tina V861 v1.1 压缩包重新解压 SDK，仅覆盖本分支 `source/`，完整编译、LYNX 烧录校验，并通过串口完成识别及一分钟 RAW 收帧验证。详情见 [干净 SDK 验证记录](docs/clean-sdk-validation.md)。

## 文件与修改范围

```text
source/                     按 SDK 原路径保存的五个完整修改文件
patches/gc2093-driver-minimal.patch  相对原始 SDK 的完整差异
test/                       RAW 测试源码、编译/运行脚本及串口传输辅助程序
docs/                       本分支干净 SDK 验证记录
source-manifest.json         五个 source 文件的 SHA-256
SHA256SUMS                  仓库交付文件校验清单
```

| SDK 相对路径 | 必要修改 |
|---|---|
| `device/config/chips/v861/configs/avaota_f2/linux-6.6-xuantie/board.dts` | GC2093 名称、地址 0xfc、MCLK1、模组供电及 MIPI0/CSI0 路由 |
| `device/config/chips/v861/configs/avaota_f2/linux-6.6-xuantie/bsp_defconfig` | 用 `CONFIG_SENSOR_GC2093_MIPI=m` 替代原 GC2083 驱动选择 |
| `openwrt/target/v861/v861-avaota_f2/defconfig` | 用 `CONFIG_PACKAGE_kmod-sensor-gc2093=y` 替代原 GC2083 模块包 |
| `openwrt/target/v861/v861-avaota_f2/modules.mk` | 增加 GC2093 模块包定义，确保完整构建将模块装入 rootfs |
| `openwrt/target/v861/v861-avaota_f2/busybox-init-base-files/etc/init.d/S00mpp` | 在 VIN 之前加载 GC2093，停止时卸载 |

SDK 原有 `bsp/drivers/vin/modules/sensor/gc2093_mipi.c` 无需修改，因此不重复收录。VIN/ISP 公共驱动、CPU 频率、TDM、TF 卡和 WiFi 配置均保持原始 SDK 默认值。本次内核未启用 TDM，不需要给 `insmod` 添加传感器参数，启动脚本自动加载模块。

模组的 **7 位 I²C 地址为 0x7e**；VIN 将设备树地址字段右移一位，因此 `sensor0_twi_addr` 填 **0xfc**。现有 RESET 为 PA14，PWDN 未单独控制；模组与接线改变时应重新核对电源和控制脚。

本分支验证驱动识别及 RAW 收帧，未交付 GC2093 专用 ISP 调校。OpenWrt 的 ISP 库选择保持原 SDK 默认值；本测试绕过 ISP，不代表 YUV 颜色或编码效果已经验证。

## 干净 SDK 完整编译（Ubuntu 主机）

先解压原始 `tina-v861-v1.1.tar.xz` 到新目录。将以下两个路径改为实际绝对路径，SDK 与仓库分开存放：

```sh
SDK_DIR=/home/ubuntu/tina-v861-v1.1
PATCH_DIR=/home/ubuntu/Downloads/avaota-f2-patches
git clone --branch gc2093-support --single-branch https://github.com/dshanpi/avaota-f2-patches.git "$PATCH_DIR"
cd "$SDK_DIR"
./build.sh autoconfig -i v861 -o openwrt -b avaota_f2 -n nor -k linux-6.6-xuantie -a riscv32
cp -a "$PATCH_DIR/source/." "$SDK_DIR/"
./build.sh loadconfig
cp openwrt/target/v861/v861-avaota_f2/defconfig openwrt/openwrt/.config
./build.sh openwrt_rootfs defconfig
./build.sh
./build.sh pack
```

仓库已存在时跳过 clone，并确认使用 `gc2093-support` 分支。首次使用 SDK 时按原厂流程处理许可提示。固件输出为 `out/v861_linux_avaota_f2_uart0_nor.img`。使用匹配本板的烧录工具烧录该整包并校验，重新上电后通过串口测试。

已有自定义设置时先审查差异，避免整文件覆盖自己的修改。也可在原始 SDK 根目录先执行 `patch --dry-run -p1 < /absolute/path/to/gc2093-driver-minimal.patch`，确认后去掉 `--dry-run` 应用补丁，再执行上述配置加载和完整构建。覆盖 `source/` 与应用补丁二选一，不要重复应用。

干净构建曾发现只有设备树和两个 defconfig 不足以把模块打包并自动加载，现已补齐 `modules.mk` 与 `S00mpp`；五文件补丁生成的结果与 `source/` 逐字节一致。

## 测试（开发板串口）

完整步骤见 [test/README.md](test/README.md)。所有板端测试命令在串口执行，115200、8N1。支持通过串口发送文本脚本把应用放入 `/tmp`，无需 ADB 或 TF 卡；有已正常工作的 TF 卡时也可离线复制文件。

```sh
dmesg | grep -i gc2093
lsmod | grep gc2093
ls -l /dev/video0
cd /tmp/gc2093-test
sh run-camera-test.sh
cat /tmp/gc2093-raw-test/exit-code.txt
```

识别成功应读到 ID 高字节 0x20、低字节 0x93。采集程序返回码为 0，且输出 `error_flags=0 sequence_gaps=0` 才表示本轮收帧通过。

原 SDK 的 TF 卡检测配置在本次实物上未识别到卡，因此本次验证采用 `/tmp` 串口传输路径。自动识别和挂载 TF 卡不属于这个 GC2093 驱动补丁的修改范围。
