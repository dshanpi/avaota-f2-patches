# V861 Avaota F2 GC2093 驱动适配

本目录适用于用户当前的 `v861-avaota_f2-tina` SDK 和图示内置 24MHz 晶振、7 位 I²C 地址 0x7e 的 GC2093 模组。GC2093 支持先沿用 SDK 已有的 1920×1080、30fps、双 Lane 线性模式。

## 配置

设备树路径：`device/config/chips/v861/configs/avaota_f2/linux-6.6-xuantie/board.dts`。

```dts
sensor0_mname = "gc2093_mipi";
sensor0_twi_cci_id = <1>;
sensor0_twi_addr = <0xfc>;
sensor0_mclk_id = <1>;
sensor0_reset = <&pio PA 14 GPIO_ACTIVE_LOW>;
sensor0_pwdn = <>;
```

四 Lane 接口上的 MIPI0/CSI0 用于本次模组，实际传感器先按两 Lane 模式输出。图示连接器的四组数据引脚不表示本次验证了四 Lane 模式。

`sensor0_twi_addr` 是 VIN 使用的地址编码。VIN 会右移一位得到 Linux I²C 地址，所以 0xfc 对应 0x7e。不得将 Linux 用户态命令的地址改成 0xfc。

内核配置需启用 `CONFIG_SENSOR_GC2093_MIPI=m`；OpenWrt 配置需启用 `CONFIG_PACKAGE_kmod-sensor-gc2093=y`。该用户 SDK 原已启用，两处不需要额外改动。

模组内置晶振。本次沿用 SDK 驱动的外部 MCLK 操作，上电时软件报告 24MHz；没有验证关闭外部 MCLK 后的行为。RESET/PWDN 的现有接线已能读 ID，不额外操作不明引脚。

## 文件用途

- `gc2093-i2c-address.patch`：当前已选 GC2093 的用户 SDK 上，本次新增的一行修正；可在 SDK 根目录执行 `patch --dry-run -p1 < ...` 后应用。
- `tested-sdk-snapshot/`：本次测试使用的驱动、设备树、内核和 OpenWrt 配置快照，路径相对 SDK 根目录。用于对照现有配置，**不是直接覆盖旧 OV13850 发布包的通用升级包**。
- `source-manifest.json`：快照 SHA-256。
- `gc2093_raw_test.c` / `gc2093_raw_test`：独立 RAW 测试程序源码和静态 musl/RISC-V32 可执行文件。
- `firmware.json`：本次固件大小、SHA-256 和编译范围。

SDK 自带的 `gc2093_mipi.c` 未修改。快照中的 OPP、TDM、其他外设及 ISP 编译选项属于用户 SDK 先前配置；本次没有把这些修改作为 GC2093 必需修改重新引入。

此分支根目录的历史 README、source 和 patches 对应 OV13850/v0.2.0，GC2093 本次资料以本目录为准。

## 编译

在已选 `avaota_f2` 的 SDK 根目录执行：

```sh
./build.sh kernel
./build.sh pack
```

本次是内核增量编译并复用已有 rootfs 打包。若在其他 SDK 中首次改变 OpenWrt 模块选项，需要完整执行 `./build.sh`，再打包；本次尚未进行新解压 SDK 的完整复编验证。

## 板端测试

固件应自动加载驱动并在日志读到 ID 0x2093，产生 `/dev/video0`。

```sh
dmesg | grep -E 'gc2093|ID_VAL'
ls /dev/video*
chmod +x /tmp/gc2093_raw_test
/tmp/gc2093_raw_test 300 /mnt/extsd/gc2093-frame.raw
```

程序输出实际协商尺寸、fourcc、stride、buffer 大小、帧时间戳、实际帧率、帧序号缺口和错误标志；保存第 11 帧。TF 挂载路径以实际 `mount` 输出为准，也可使用 `/tmp` 保存单帧。

本次关注芯片识别与 RAW 收帧。没有为 GC2093 冒用 OV13850 的 ISP 效果文件；专用 ISP 标定、YUV 图像质量、HDR 和其他帧率尚需后续验证。
