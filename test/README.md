# GC2093 RAW 驱动测试

本目录验证驱动识别、RAW收帧和帧率，不启动ISP或编码器。

## 1. 在SDK主机编译应用

```sh
sh test/build-test.sh /home/ubuntu/tina-v861-v1.1
```

生成 `test/gc2093_raw_test`，使用SDK自带的RISC-V32 musl交叉编译器静态链接。如工具链目录不同，可设置 `CC=/absolute/path/to/riscv32-unknown-linux-musl-gcc`。

## 2. 传入板端

先确认ADB连接到待测摄像头板，再执行：

```sh
adb push test /tmp/gc2093-test
adb shell 'chmod +x /tmp/gc2093-test/gc2093_raw_test'
adb shell 'sh /tmp/gc2093-test/run-camera-test.sh'
```

默认配置为1800帧、保存一帧到 `/mnt/extsd/gc2093-raw-test`，约一分钟。请先通过 `mount` 确认普通TF卡已挂载到 `/mnt/extsd`；若挂载在其他路径，请修改 `raw-1080p30.conf` 中的 `OUTPUT_DIR`。没有TF卡时可以使用 `/tmp/gc2093-raw-test`，单帧约4MiB，重启后会丢失。

传输目录后，板端文件结构应为 `/tmp/gc2093-test/gc2093_raw_test`、`run-camera-test.sh` 和 `raw-1080p30.conf`；若目的目录之前存在，请检查ADB实际生成的目录层级。

测试应用也可以直接执行：

```sh
/tmp/gc2093-test/gc2093_raw_test 1800 /mnt/extsd/gc2093-frame.raw
```

传入保存路径时，采集到第11帧会保存一帧；省略该参数仅测收帧，例如 `gc2093_raw_test 1800`。设置 `SAVE_FRAME=0` 可由脚本进行不保存图片的测试。

## 3. 判断结果

应用打印实际协商尺寸、fourcc、帧序号、时间戳和帧长度，结束打印：

```text
RESULT frames=1800 elapsed=... fps=... error_flags=0 sequence_gaps=0
```

帧率按 `(帧数−1)/(末帧时间戳−首帧时间戳)` 计算。返回0表示采集完成且未出现ERROR标志和序号缺口；返回2表示存在帧错误或序号缺口；返回1表示打开设备、配置、等待帧或其他操作失败。脚本保留应用返回码并写入 `exit-code.txt`，不会把失败输出成通过。

本程序固定使用 `/dev/video0`、1920×1080 RAW10、30fps、三个MMAP缓冲。该SDK实际返回每帧4177920字节，对应1920×1088×2，包含高度对齐填充；显示图像时应只取可见1080行。

GC2093驱动报告未翻转时的Bayer顺序为RGGB；V4L2请求fourcc BG10不等于驱动已重排Bayer像素。RAW文件不能直接当作JPEG或YUV打开。

## 既有板端实测

- 1800帧不保存测试：30.0061fps，ERROR标志0，序号缺口0。
- 300帧保存测试：29.9061fps，ERROR标志0，序号缺口1；已如实保留该结果，没有宣称所有测试零丢帧。

这些是本次驱动适配的既有实测记录。新增脚本只负责调用同一采集程序和保存返回码，不代表又完成了新一轮一分钟长测。
