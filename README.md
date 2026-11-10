# V861 Avaota F2 GC2093 驱动最小修改

本分支仅存放GC2093驱动适配所需的最小增量。ISP效果参数、照片和采集应用属于其他调试资料，不纳入本分支。

## 目录结构

```text
source/                     补丁对应的完整修改后源码，保持SDK原路径
patches/                    GC2093驱动最小差异补丁
test/                       RAW采集应用源码、构建脚本、运行脚本和配置
source-manifest.json         source文件SHA-256清单
SHA256SUMS                  交付文件校验值
```

`source/` 与主分支采用相同布局，保存驱动适配所需的五个完整板级文件、选中配置及模块集成：

| SDK相对路径 | 用途 |
|---|---|
| `device/config/chips/v861/configs/avaota_f2/linux-6.6-xuantie/board.dts` | GC2093地址、总线、路由及复位配置 |
| `device/config/chips/v861/configs/avaota_f2/linux-6.6-xuantie/bsp_defconfig` | `CONFIG_SENSOR_GC2093_MIPI=m`，编译GC2093内核模块 |
| `openwrt/target/v861/v861-avaota_f2/defconfig` | `CONFIG_PACKAGE_kmod-sensor-gc2093=y`，把模块打包进固件并参与自动加载 |
| `openwrt/target/v861/v861-avaota_f2/modules.mk` | 定义GC2093 OpenWrt内核模块包 |
| `openwrt/target/v861/v861-avaota_f2/busybox-init-base-files/etc/init.d/S00mpp` | 在VIN之前加载GC2093，停止时卸载 |

SDK原有 `gc2093_mipi.c` 没有修改，因此不重复收录。干净SDK验证发现，仅提供设备树和两个defconfig会遗漏OpenWrt包定义与BusyBox启动加载；现已补齐。五个文件均从原始SDK基线生成，只保留GC2093所需差异，TDM模式和CPU频率保持原SDK默认值。

可以按SDK原路径覆盖 `source/` 五个文件，或在原始SDK根目录应用 `patches/gc2093-driver-minimal.patch`，二选一。已有自定义配置时应先审查差异，避免覆盖自己的设置。

OpenWrt的ISP效果选择保持原SDK默认值，本分支只选择GC2093内核驱动包，RAW测试不启动ISP。

测试方法见 [test/README.md](test/README.md)。测试应用源码和脚本不属于需要覆盖到SDK中的驱动修改；预编译应用在SDK主机执行 `test/build-test.sh` 后生成。

## 本次实际修改

SDK已有 `bsp/drivers/vin/modules/sensor/gc2093_mipi.c`，无需修改寄存器驱动。板级适配包括：I²C1、MIPI0/CSI0、MCLK1、模组供电及复位选择，驱动编译和固件打包，以及BusyBox启动加载。

本模组的**7位I²C地址是0x7e**。VIN将设备树的该字段右移一位，因此 `sensor0_twi_addr` 填0xfc；用户态i2ctransfer仍使用0x7e。

## 干净SDK复现（Ubuntu SDK主机）

先从原始 `tina-v861-v1.1.tar.xz` 解压新SDK，并取得本分支。以下假设仓库在 `$PATCH_DIR`、SDK在 `$SDK_DIR`，两者均设置为实际绝对路径：

```sh
SDK_DIR=/home/ubuntu/tina-v861-v1.1
PATCH_DIR=/home/ubuntu/Downloads/avaota-f2-patches
cd "$SDK_DIR"
./build.sh autoconfig -i v861 -o openwrt -b avaota_f2 -n nor -k linux-6.6-xuantie -a riscv32
cp -a "$PATCH_DIR/source/." "$SDK_DIR/"
./build.sh loadconfig
cp openwrt/target/v861/v861-avaota_f2/defconfig openwrt/openwrt/.config
./build.sh openwrt_rootfs defconfig
./build.sh
./build.sh pack
```

完整编译使内核驱动选择和OpenWrt模块打包配置共同生效。SDK首次配置时如出现原厂许可提示，请按原SDK流程处理。固件输出为 `out/v861_linux_avaota_f2_uart0_nor.img`。这套流程正在本次新解压SDK上完整验证；在验证完成前不将其列为已通过。

若采用补丁方式，请先在原始SDK根目录执行 `patch --dry-run -p1 < /path/to/gc2093-driver-minimal.patch`，确认后应用，再执行相同的完整构建流程。已经覆盖过 `source/` 的SDK不需要再套同一补丁。

## 测试（开发板串口）

先在Ubuntu主机编译测试应用，将整个 `test` 目录复制到普通TF卡根目录并改名为 `gc2093-test`，再插入开发板。详见 [test/README.md](test/README.md)。

串口115200、8N1进入Linux命令行后，确认TF卡挂载在 `/mnt/extsd`，执行：

```sh
cd /mnt/extsd/gc2093-test
chmod +x gc2093_raw_test
sh run-camera-test.sh
```

测试结果在 `/mnt/extsd/gc2093-raw-test/`。若挂载路径不同，修改命令中的路径及测试配置的 `OUTPUT_DIR`。板端命令统一直接在串口执行，SDK编译命令在Ubuntu主机执行。

## 适配配置核对

以下是驱动适配所需配置；选择项与模块集成一并保存在 `source/` 供外发使用：

```dts
sensor0_mname = "gc2093_mipi";
sensor0_twi_cci_id = <1>;
sensor0_mclk_id = <1>;
sensor0_reset = <&pio PA 14 GPIO_ACTIVE_LOW>;
sensor0_pwdn = <>;
```

- 内核：`CONFIG_SENSOR_GC2093_MIPI=m`。
- OpenWrt：`CONFIG_PACKAGE_kmod-sensor-gc2093=y`。
- 使用四Lane接口中的MIPI0/CSI0，GC2093本次实际输出为双Lane1080P30线性模式。
- 模组内置24MHz晶振，现有RESET/PWDN接线可正常读ID，不额外改动控制脚。

尚未选择这些配置的干净SDK，需要先核对板级路由、供电复位和模块选项；本分支不宣称这一行补丁能替代所有板级配置。

## 驱动验证

- 内核编译、固件打包、LYNX烧录与校验成功。
- 启动自动读取GC2093芯片ID `0x2093`，产生 `/dev/video0` 等节点，无需附加模块参数。
- 1080P30 RAW一分钟采集1800帧，实测30.0061fps，帧错误标志0、序号缺口0。
- 较早300帧短测有1次序号缺口，随后一分钟复测未复现；未将其描述为全部测试零丢帧。

验证范围为驱动识别和RAW收帧。ISP调校、YUV/编码和其他模式不属于本分支交付范围。
