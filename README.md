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

`source/` 与主分支采用相同布局，保存驱动适配所需的完整板级文件和选中配置：

| SDK相对路径 | 用途 |
|---|---|
| `device/config/chips/v861/configs/avaota_f2/linux-6.6-xuantie/board.dts` | GC2093地址、总线、路由及复位配置 |
| `device/config/chips/v861/configs/avaota_f2/linux-6.6-xuantie/bsp_defconfig` | `CONFIG_SENSOR_GC2093_MIPI=m`，编译GC2093内核模块 |
| `openwrt/target/v861/v861-avaota_f2/defconfig` | `CONFIG_PACKAGE_kmod-sensor-gc2093=y`，把模块打包进固件并参与自动加载 |

SDK原有 `gc2093_mipi.c` 没有修改，因此不重复收录。此前只收录设备树会遗漏外发时必要的选中配置，现在补齐这两个配置文件。它们是本次已验证SDK的完整配置，保留用户原SDK既有设置；GC2093选择项原已启用，不能把完整文件中所有选项都计作此次新增修改。

注意，OpenWrt配置里的 `CONFIG_ov13850=y` 是原SDK已有的ISP效果库编译选项，**不是**内核驱动选择；传感器驱动包实际由 `CONFIG_PACKAGE_kmod-sensor-gc2093=y` 选择。本分支只交付驱动适配，未为了删除历史ISP选项而扩大修改范围。

相同SDK/板级配置基线下，可按SDK原路径覆盖 `source/` 中的三个文件。已有其他板级定制时，应合并设备树和上述两个GC2093选择项，避免整文件覆盖自己的设置。

`patches/gc2093-driver-minimal.patch` 保留为**已选中GC2093的用户SDK上仅修正地址的一行增量**，它不重复包含原已启用的驱动配置。尚未选中驱动时，不能只套用这个地址补丁而漏掉两个配置项。

测试方法见 [test/README.md](test/README.md)。测试应用源码和脚本不属于需要覆盖到SDK中的驱动修改；预编译应用在SDK主机执行 `test/build-test.sh` 后生成。

## 本次实际修改

SDK已有 `bsp/drivers/vin/modules/sensor/gc2093_mipi.c`，已能驱动这颗模组，本次无需修改或复制该文件。

当前用户SDK已配置GC2093、I²C1、MIPI0/CSI0和模块自动加载。本次修正一个设备树字段：

文件：`device/config/chips/v861/configs/avaota_f2/linux-6.6-xuantie/board.dts`

```diff
- sensor0_twi_addr = <0x7e>;
+ sensor0_twi_addr = <0xfc>;
```

本模组的**7位I²C地址是0x7e**。VIN将设备树的该字段右移一位，因此必须填0xfc；填0x7e时会错误访问0x3f。用户态i2ctransfer仍使用0x7e，不使用0xfc。

## 应用（Ubuntu SDK主机）

先选择 `v861-avaota_f2-tina` 配置，将仓库 `source/` 的文件按原相对路径复制到匹配的SDK，或合并对应设置。随后在SDK根目录完整编译并打包，使内核模块选择和OpenWrt模块打包配置都生效：

```sh
./build.sh
./build.sh pack
```

如果当前SDK已经选中并打包了GC2093，只需要地址修正，也可以使用一行补丁：

```sh
patch --dry-run -p1 < /path/to/gc2093-driver-minimal.patch
patch -p1 < /path/to/gc2093-driver-minimal.patch
./build.sh kernel
./build.sh pack
```

该地址补丁基线为本次用户正在使用、已选好GC2093的SDK，差异仅1处、1行。若设备树已经是0xfc，不必重复应用。

## 测试（开发板串口）

先在Ubuntu主机编译测试应用，将整个 `test` 目录复制到普通TF卡根目录并改名为 `gc2093-test`，再插入开发板。详见 [test/README.md](test/README.md)。

串口115200、8N1进入Linux命令行后，确认TF卡挂载在 `/mnt/extsd`，执行：

```sh
cd /mnt/extsd/gc2093-test
chmod +x gc2093_raw_test
sh run-camera-test.sh
```

测试结果在 `/mnt/extsd/gc2093-raw-test/`。若挂载路径不同，修改命令中的路径及测试配置的 `OUTPUT_DIR`。板端命令统一直接在串口执行，SDK编译命令在Ubuntu主机执行。

## 原SDK已具备的配置

以下是驱动适配所需配置；原已启用的选择项也一并保存在 `source/` 供外发使用：

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
