# GC2093 ISP 初始出图配置（2026-11-10）

这是一套在 V861 Avaota F2 上实际输出 1920×1080 NV21 的初始 ISP 配置，用于继续调试，尚未完成模组 ISP 标定。当前镜头主要拍摄窗框、天花板和灯具；照片可辨认场景，存在混合光源色差、高光饱和与暗部噪声，不能作为色彩准确性的结论。

## 参考与转换

参考仓库：<https://github.com/kendryte/k230_linux_sdk/tree/dad154b3a29d895dc28f95fa1f1eb4bbba2ee7be/buildroot-overlay/package/vvcam>。

- K230 XML `configs/gc2093-1920x1080.xml`：五组光源的 `manualWB` 和 `manualccMatrix`，以及 BLS 数据 `[64,64,64,64]`。
- K230 `src/gc2093.c`：未翻转时 RGGB、曝光增益反馈延迟两帧。沿用本板已验证的全志 GC2093 寄存器表。
- V861 AWB 光源表需要白场 R/G、B/G 统计比值，因此以 K230 白平衡增益的倒数换算成 Q8 比值，按五种光源建立表；光源色温采用初始近似对应值。
- CCM 的浮点系数转换到 Q8。A/TL84/D65 三组分别作为暖光、中间光、日光初始矩阵。
- V861 自带 GC2083 ISP610 配置提供平台通用处理结构与曲线初值，复制成独立 GC2093 参数。黑电平继续使用基线的 `-64`，与 K230 黑电平数值进行核对；没有据此宣称两个 ISP 的全部黑电平定义已严格等效标定。
- 曝光表限制在30fps范围，初始最大模拟增益16倍，曝光与增益反馈延迟设为两帧。
- 镜头相关 LSC/MSC 保持关闭；K230 的 AWB GMM、网格阴影表和硬件降噪参数没有直接搬用。

转换记录见 `isp-reference-map.json`，K230 文件提交和哈希见 `k230-reference-manifest.json`。

## 新增源码

`source/` 中相对 SDK 路径：

1. `platform/allwinner/vision/libAWIspApi/isp_mpp/isp_v861/libisp/isp_cfg/SENSOR_H/gc2093/gc2093_mipi_k230_reference_isp610.h`：独立初始参数。
2. 同目录上层的 `isp_ini_parse.c`：增加 `gc2093_mipi / 1920×1080 / 30fps / 线性 / 彩色` 匹配项，效果名 `gc2093_k230_reference_bringup`。
3. 同目录的 `tina.mk`：支持 `SENSOR_GC2093` 编译宏。
4. `openwrt/package/allwinner/vision/libAWIspApi/machinfo/v861/libisp_config.mk`：已启用 GC2093 内核模块包时，将GC2093加入ISP构建，不新增一个重复的传感器选择项。

`gc2093-isp-from-driver-4bc5b90.patch` 是相对当前用户 SDK 中原有 OV13850 ISP 集成的增量，不能保证适用于任意其他 SDK 版本；先用 `patch --dry-run -p1` 检查。四文件哈希见 `source-manifest.json`。

## 实测

基于前一轮已烧录的 GC2093 地址修正版内核，当前启动ID `7741456d-6d5e-4712-a688-25e46f1151e9`，通过串口与虚拟机 ADB 双重核对。

两次独立启动与退出采集程序均成功，配置分别运行20秒、15秒，各保存一帧 3110400 字节的 1920×1080 NV21。保存延迟到应用收到181帧之后，VI配置另有启动丢弃90帧，给AE/AWB初始收敛留出时间。

日志确认：

```text
find gc2093_mipi_1920_1080_30_0 [gc2093_k230_reference_bringup] isp config
```

第一帧Y均值98.10，第二帧97.79；这只是当前固定场景的重复结果，不是色卡标定或动态AE验证。此次程序没有新增帧率统计；30fps RAW统计见上层驱动验证记录，不能等同于本轮ISP实测帧率。

仍有启动期一次AEWB统计无效、驱动不支持flip/temperature查询以及停止期 `isp0 device is NULL` 的TDM回调日志；均记录在 `test/` 原始日志中。两次程序退出码为0且保存图像成功，不能将这些事实表述成“所有错误已修复”。

照片：`photo.png` 为NV21按BT.709限幅转换后的显示预览，未额外做白平衡、曝光或色彩修改。

## 当前板端使用

TF卡目录 `/mnt/extsd/gc2093-isp/` 存放已链接新参数的 `sample_gc2093_isp`、`isp-test.conf` 和首张 `frame.nv21`。

```sh
LD_LIBRARY_PATH=/usr/lib/eyesee-mpp:/usr/lib \
  /mnt/extsd/gc2093-isp/sample_gc2093_isp \
  -path /mnt/extsd/gc2093-isp/isp-test.conf
```

第二张照片、配置和日志在 `/mnt/extsd/gc2093-isp-photo/`。

本SDK以静态方式把ISP库链接进应用。因此这轮通过替换应用验证生效，未重新烧录固件；旧驱动测试固件里的业务程序不会自动带上本次新参数。后续业务程序也须重新链接新的ISP库。

## 复编

SDK已配置为avaota_f2并具备GC2093驱动时：

```sh
./build.sh openwrt_rootfs package/subpackage/allwinner/vision/libAWIspApi/compile V=s
./build.sh openwrt_rootfs package/subpackage/allwinner/eyesee-mpp/middleware/compile V=s
```

本次额外测试补丁 `test/sample_virvi-delayed-save.patch` 修正空保存管理器指针处理并延迟保存图片，属于测试应用修改。它不改变VIN/ISP公共驱动。已生成的应用与应用源代码补丁一同交付。

尚未进行全新SDK复编、固件集成复测、编码验证、标准色卡和多光源标定。当前交付目标是可重复出图。
