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

`source/` 与主分支采用相同布局，当前只放实际修改的 `board.dts`。SDK原有 `gc2093_mipi.c` 没有修改，因此不重复收录。完整设备树保留原用户SDK已有设置；本次增量仍只有地址一行，请结合补丁审查，避免将既有设置当成本次新增修改。

可以按原SDK路径覆盖 `source/` 中的文件，或应用 `patches/` 中的补丁，二选一即可。整文件替换适用于相同SDK/板级配置基线；其他版本先审查差异。

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

## 应用

在当前已选择 `v861-avaota_f2-tina` 并启用GC2093的SDK根目录：

```sh
patch --dry-run -p1 < /path/to/gc2093-driver-minimal.patch
patch -p1 < /path/to/gc2093-driver-minimal.patch
./build.sh kernel
./build.sh pack
```

补丁基线为本次用户正在使用、已选好GC2093的SDK，完整差异仅1处、1行。若设备树已经是0xfc，不必重复应用。

## 原SDK已具备的配置

以下是适用前提，不计入本次新增修改：

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
