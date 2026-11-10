# V861 Avaota F2 GC2093 最小适配增量

分支：`gc2093-support`。适用配置：`v861-avaota_f2-tina`，模组内置24MHz晶振、7位I²C地址0x7e。本目录针对本次用户正在使用、已经启用GC2093驱动和模块包的SDK进行最小收敛；不是针对任意全新SDK的完整替换包。

## 必需修改：3个文件路径

| 文件 | 修改内容 |
|---|---|
| `device/config/chips/v861/configs/avaota_f2/linux-6.6-xuantie/board.dts` | 本次已有GC2093配置中的地址0x7e改为0xfc，共1行 |
| `platform/allwinner/vision/libAWIspApi/isp_mpp/isp_v861/libisp/isp_cfg/isp_ini_parse.c` | 加载GC2093头文件，增加1080P30线性配置匹配，复制公共默认参数到当前ISP实例后应用GC2093覆盖值 |
| 同目录 `SENSOR_H/gc2093/gc2093_mipi_k230_reference_isp610.h` | 新增46行、1849字节的参数覆盖代码 |

完整增量：`patches/gc2093-minimal.patch`，共96行（含差异头和上下文）。`source/` 另存新增头文件便于阅读，应用补丁时不需要重复复制。

复用SDK自带GC2083 ISP610公共默认处理表，只覆盖GC2093的AE、AWB光源比值和CCM参数。原始默认表保持不变，先复制到当前ISP实例的私有配置再覆盖，因此不会改动其他摄像头正在使用的全局参数。

已撤回上一版两处构建集成修改（`isp_cfg/tina.mk`和`machinfo/v861/libisp_config.mk`），并移除了重复的大参数表、完整SDK文件快照和旧OV13850/TF WiFi交付文件在本分支工作树中的副本；这些既有功能仍保留在仓库main分支及历史中。本分支不改变GC2093寄存器驱动、VIN公共代码、分区和其他外设配置。

## 应用前提与方法

当前SDK已经具备以下配置：

```dts
sensor0_mname = "gc2093_mipi";
sensor0_twi_cci_id = <1>;
sensor0_mclk_id = <1>;
sensor0_reset = <&pio PA 14 GPIO_ACTIVE_LOW>;
sensor0_pwdn = <>;
```

四Lane连接器使用MIPI0/CSI0，本次传感器输出模式是双Lane1080P30。内核`CONFIG_SENSOR_GC2093_MIPI=m`，OpenWrt `CONFIG_PACKAGE_kmod-sensor-gc2093=y`。这些由用户原SDK预先选好，本补丁不再重复修改。

若使用尚未配置GC2093的干净SDK，必须先核对上述板级路由、供电复位以及模块选项。本分支没有宣称完成干净SDK全量复编验证。

在适用SDK根目录执行：

```sh
patch --dry-run -p1 < /path/to/gc2093-minimal.patch
patch -p1 < /path/to/gc2093-minimal.patch
./build.sh kernel
./build.sh openwrt_rootfs package/subpackage/allwinner/vision/libAWIspApi/compile V=s
./build.sh openwrt_rootfs package/subpackage/allwinner/eyesee-mpp/middleware/compile V=s
```

VIN将设备树地址右移一位，所以0xfc对应用户态I²C地址0x7e。不要把i2ctransfer地址写成0xfc。固件烧录与分区集成应按本SDK原流程进行；这一轮ISP精简验证通过传入应用完成，没有重新烧录。

## ISP数据来源及验证

参考 K230 SDK [GC2093资料](https://github.com/kendryte/k230_linux_sdk/tree/dad154b3a29d895dc28f95fa1f1eb4bbba2ee7be/buildroot-overlay/package/vvcam)，原始文件与哈希见`docs/k230-reference-manifest.json`。

白平衡增益取倒数形成Q8白场R/G、B/G比值，CCM转换为Q8，镜头阴影校正保持关闭；具体记录在`docs/reference-map.json`。这些是初始出图参数，不是针对本模组完成的色卡和多光源标定。

- **参数等价性**：在板端逐字节比较旧完整头文件方案和新46行方案生成的全部163984字节参数，结果完全一致。见`docs/parameter-compare.log`。
- **构建**：精简后的ISP库和采集应用重新编译成功；补丁反向dry-run通过。
- **实拍**：精简版应用退出码0，保存1920×1080 NV21共3110400字节；预览见`docs/photo.png`。
- **驱动既有结果**：前一轮1080P30 RAW一分钟1800帧，实测30.0061fps，错误标志0、序号缺口0；早先300帧短测有1次序号缺口。本轮未重新把RAW帧率作为ISP帧率验证。

## 测试应用

`tests/sample_virvi-delayed-save.patch`是**可选测试补丁**，属于测试程序修改，不计入上述3个生产文件：避免空保存管理器指针处理，并将保存延迟到181帧后，给AE/AWB收敛时间。生产应用不需要为加载GC2093参数而应用这个补丁。

本SDK的ISP库静态链接进应用；需要重新链接业务程序。仅复制头文件或烧录旧驱动测试固件，不会更新旧应用内的ISP参数。

当前TF卡已经保存重新链接的测试程序和配置：

```sh
LD_LIBRARY_PATH=/usr/lib/eyesee-mpp:/usr/lib \
  /mnt/extsd/gc2093-minimal/sample_gc2093_isp \
  -path /mnt/extsd/gc2093-minimal/photo.conf
```

`tests/photo.conf`为对应配置，照片保存到`/mnt/extsd/gc2093-minimal/photo.nv21`。

当前图像能辨认场景，但仍有混合光源色差和暗部噪声。启动期统计无效、flip/temperature查询不支持、停止期TDM回调警告尚未解决，见`docs/validation.md`。不宣称ISP标定完成或所有SDK日志错误已消除。
