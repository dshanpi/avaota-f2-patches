# GC2093 1080P30 RAW 串口测试

测试固定使用 `/dev/video0`、1920×1080 RAW10、30fps、三个 MMAP 缓冲；不启动 ISP 和编码器。

## 1. Ubuntu SDK 主机编译

在本分支仓库根目录执行，SDK 路径改为实际值：

```sh
sh test/build-test.sh /home/ubuntu/tina-v861-v1.1
python3 test/make-serial-upload.py
```

使用 SDK 自带 RISC-V32 musl 工具链静态链接，生成 `test/gc2093_raw_test`。工具链位置不同时可通过 `CC=/absolute/path/to/riscv32-unknown-linux-musl-gcc` 指定。

## 2. 通过串口传入 /tmp（本次实测方式）

串口 115200、8N1，先确认已进入 Linux shell。将上一步生成的 `test/uart-upload.txt` 复制到串口终端所在电脑，使用终端的“发送文本文件”功能发送内容；使用 LF 换行，每行发送间隔至少 20ms。这是供 shell 执行的文本，不要用二进制文件传输模式发送它。

生成器只使用 `mkdir`、shell 内建 `printf`、`chmod` 和 `md5sum`，不依赖 `stty`、base64、ZMODEM 或 ADB。脚本逐段重建应用、运行脚本和配置到 `/tmp/gc2093-test`。等到输出 `GC2093_UPLOAD_DONE` 后，检查终端打印的应用 MD5 与主机生成器打印的预期值一致；不一致时重新完整发送，不要运行残缺程序。

然后在开发板串口执行：

```sh
dmesg | grep -i gc2093
lsmod | grep gc2093
ls -l /dev/video0
cd /tmp/gc2093-test
sh run-camera-test.sh
cat /tmp/gc2093-raw-test/exit-code.txt
tail -n 3 /tmp/gc2093-raw-test/capture.log
ls -l /tmp/gc2093-raw-test/frame.raw
```

默认采集 1800 帧，约一分钟；停流后保存最后一帧，不在采集中同步写文件占用缓冲区。结果保存在 `/tmp/gc2093-raw-test/`，断电丢失；需要保留时先复制日志和 RAW 文件。

## 3. 可选：用已经正常工作的 TF 卡复制

在电脑上将编译后的 `test` 目录复制到 TF 卡根目录，改名为 `gc2093-test`，至少包含可执行文件、`run-camera-test.sh` 和 `raw-1080p30.conf`。安全弹出后插入开发板，确认卡已挂载到 `/mnt/extsd`。

在开发板串口执行：

```sh
mount
ls /mnt/extsd/gc2093-test
cd /mnt/extsd/gc2093-test
chmod +x gc2093_raw_test
sh run-camera-test.sh
cat /mnt/extsd/gc2093-raw-test/exit-code.txt
```

仓库配置文件的 `OUTPUT_DIR` 默认为 `/mnt/extsd/gc2093-raw-test`；串口传输生成器仅在传输副本中将其改为 `/tmp/gc2093-raw-test`。其他挂载路径自行调整命令及 `OUTPUT_DIR`。本分支不修改原 SDK 的 TF 卡检测配置；卡未识别时请用第 2 节。

## 4. 结果与 RAW 格式

成功示例的关键字段为：

```text
RESULT frames=1800 elapsed=... fps=... error_flags=0 sequence_gaps=0
Capture exit code: 0
```

帧率按 `(帧数−1)/(末帧时间戳−首帧时间戳)` 计算。返回 0：完成且无 ERROR 标志或序号缺口；返回 2：有帧错误或缺口；返回 1：设备、配置、等待帧或保存失败。脚本原样保留返回码，不将失败输出为通过。设置 `SAVE_FRAME=0` 可只测收帧。

单帧实际为 4177920 字节，即 1920×1088×2，包含高度对齐；可见图像取前 1080 行。驱动未翻转时报告的 Bayer 顺序为 RGGB；请求 BG10 不代表已经重排像素。RAW 不能直接当作 JPEG 或 YUV 打开。

实测结果及首轮缺口记录见 [干净 SDK 验证记录](../docs/clean-sdk-validation.md)，完整串口日志在 [results/20261110](results/20261110)。本次一分钟测试不能替代长时间或 ISP/编码验证。
