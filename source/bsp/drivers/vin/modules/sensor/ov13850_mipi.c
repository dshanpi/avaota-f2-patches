/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * A V4L2 driver for ov13850 Raw cameras.
 *
 * Copyright (c) 2017 by Allwinnertech Co., Ltd.  http:
 *
 * Authors:  Zhao Wei <zhaowei@allwinnertech.com>
 *    Yang Feng <yangfeng@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "../../utility/vin_log.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/videodev2.h>
#include <linux/clk.h>
#include <media/v4l2-device.h>
#include <media/v4l2-mediabus.h>
#include <linux/io.h>
#include "camera.h"
#include "sensor_helper.h"

MODULE_AUTHOR("lwj");
MODULE_DESCRIPTION("A low-level driver for ov13850 R2A sensors");
MODULE_LICENSE("GPL");

#define MCLK              (24*1000*1000)
static int ov13850_sensor_vts = 3333;

/*
 * Our nominal (default) frame rate.
 */

#define SENSOR_FRAME_RATE 30

/*
 * SDK 8-bit CCI address 0x20 (Linux 7-bit address 0x10).
 */
#define I2C_ADDR 0x20
#define SENSOR_NAME "ov13850_mipi"

/*
 * OV13850 R2A, 24 MHz MCLK, four-lane RAW10.
 * Common initialization derived from the module vendor K230 reference;
 * native 4K25 and 1440p/1080p/720p 30/60 fps modes are exposed.
 */

static struct regval_list sensor_default_regs[] = {
	{0x0100, 0x00},
	{0x0102, 0x01},
	{0x0103, 0x01},
	{REG_DLY, 5},
	{0x0300, 0x00},
	{0x0301, 0x00},
	{0x0302, 0x32},
	{0x0303, 0x00},
	{0x030a, 0x00},
	{0x300f, 0x11},
	{0x3010, 0x01},
	{0x3011, 0x76},
	{0x3012, 0x41},
	{0x3013, 0x12},
	{0x3014, 0x11},
	{0x301f, 0x03},
	{0x3106, 0x00},
	{0x3210, 0x47},
	/* Allow the newly programmed internal clocks to settle before AEC writes. */
	{REG_DLY, 5},
	{0x3500, 0x00},
	{0x3501, 0xfa},
	{0x3502, 0x00},
	{0x3506, 0x00},
	{0x3507, 0x00},
	{0x3508, 0x00},
	{0x350a, 0x00},
	{0x350b, 0x80},
	{0x350e, 0x00},
	{0x350f, 0x40},
	{0x351a, 0x00},
	{0x351b, 0x10},
	{0x351c, 0x00},
	{0x351d, 0x20},
	{0x351e, 0x00},
	{0x351f, 0x40},
	{0x3520, 0x00},
	{0x3521, 0x80},
	{0x3600, 0xc0},
	{0x3601, 0xfc},
	{0x3602, 0x02},
	{0x3603, 0x78},
	{0x3604, 0xb1},
	{0x3605, 0xb5},
	{0x3606, 0x73},
	{0x3607, 0x07},
	{0x3609, 0x40},
	{0x360a, 0x30},
	{0x360b, 0x91},
	{0x360c, 0x09},
	{0x360f, 0x02},
	{0x3611, 0x10},
	{0x3612, 0x27},
	{0x3613, 0x33},
	{0x3615, 0x0c},
	{0x3616, 0x0e},
	{0x3641, 0x02},
	{0x3660, 0x82},
	{0x3668, 0x54},
	{0x3669, 0x00},
	{0x366a, 0x3f},
	{0x3667, 0xa0},
	{0x3702, 0x40},
	{0x3703, 0x44},
	{0x3704, 0x2c},
	{0x3705, 0x01},
	{0x3706, 0x15},
	{0x3707, 0x44},
	{0x3708, 0x3c},
	{0x3709, 0x1f},
	{0x370a, 0x27},
	{0x370b, 0x3c},
	{0x3720, 0x55},
	{0x3722, 0x84},
	{0x3728, 0x40},
	{0x372a, 0x00},
	{0x372b, 0x02},
	{0x372e, 0x22},
	{0x372f, 0x90},
	{0x3730, 0x00},
	{0x3731, 0x00},
	{0x3732, 0x00},
	{0x3733, 0x00},
	{0x3710, 0x28},
	{0x3716, 0x03},
	{0x3718, 0x10},
	{0x3719, 0x0c},
	{0x371a, 0x08},
	{0x371c, 0xfc},
	{0x3748, 0x00},
	{0x3760, 0x13},
	{0x3761, 0x33},
	{0x3762, 0x86},
	{0x3763, 0x16},
	{0x3767, 0x24},
	{0x3768, 0x06},
	{0x3769, 0x45},
	{0x376c, 0x23},
	{0x376f, 0x80},
	{0x3773, 0x06},
	{0x3d84, 0x00},
	{0x3d85, 0x17},
	{0x3d8c, 0x73},
	{0x3d8d, 0xbf},
	{0x3800, 0x00},
	{0x3801, 0x08},
	{0x3802, 0x00},
	{0x3803, 0x04},
	{0x3804, 0x10},
	{0x3805, 0x97},
	{0x3806, 0x0c},
	{0x3807, 0x4b},
	{0x3808, 0x08},
	{0x3809, 0x40},
	{0x380a, 0x06},
	{0x380b, 0x20},
	{0x380c, 0x12},
	{0x380d, 0xc0},
	{0x380e, 0x06},
	{0x380f, 0x80},
	{0x3810, 0x00},
	{0x3811, 0x04},
	{0x3812, 0x00},
	{0x3813, 0x02},
	{0x3814, 0x11},
	{0x3815, 0x11},
	{0x3820, 0x00},
	{0x3821, 0x04},
	{0x3823, 0x00},
	{0x3826, 0x00},
	{0x3827, 0x02},
	{0x3834, 0x00},
	{0x3835, 0x1c},
	{0x3836, 0x08},
	{0x3837, 0x02},
	{0x4000, 0xf1},
	{0x4001, 0x00},
	{0x4006, 0x04},
	{0x4007, 0x04},
	{0x400b, 0x0c},
	{0x4011, 0x00},
	{0x401a, 0x00},
	{0x401b, 0x40},
	{0x401c, 0x00},
	{0x401d, 0x00},
	{0x4020, 0x00},
	{0x4021, 0xe4},
	{0x4022, 0x04},
	{0x4023, 0xd7},
	{0x4024, 0x05},
	{0x4025, 0xbc},
	{0x4026, 0x05},
	{0x4027, 0xbf},
	{0x4028, 0x00},
	{0x4029, 0x02},
	{0x402a, 0x04},
	{0x402b, 0x08},
	{0x402c, 0x02},
	{0x402d, 0x02},
	{0x402e, 0x0c},
	{0x402f, 0x08},
	{0x403d, 0x2c},
	{0x403f, 0x7f},
	{0x4041, 0x07},
	{0x4500, 0x82},
	{0x4501, 0x3c},
	{0x458b, 0x00},
	{0x459c, 0x00},
	{0x459d, 0x00},
	{0x459e, 0x00},
	{0x4601, 0x83},
	{0x4602, 0x22},
	{0x4603, 0x01},
	{0x4800, 0x24},
	{0x4837, 0x0d},
	{0x4d00, 0x04},
	{0x4d01, 0x42},
	{0x4d02, 0xd1},
	{0x4d03, 0x90},
	{0x4d04, 0x66},
	{0x4d05, 0x65},
	{0x4d0b, 0x00},
	{0x5000, 0x0e},
	{0x5001, 0x01},
	{0x5002, 0x07},
	{0x5013, 0x40},
	{0x501c, 0x00},
	{0x501d, 0x10},
	{0x510f, 0xfc},
	{0x5110, 0xf0},
	{0x5111, 0x10},
	{0x536d, 0x02},
	{0x536e, 0x67},
	{0x536f, 0x01},
	{0x5370, 0x4c},
	{0x5400, 0x00},
	{0x5401, 0x61},
	{0x5402, 0x00},
	{0x5403, 0x00},
	{0x5404, 0x00},
	{0x5405, 0x40},
	{0x540c, 0x05},
	{0x5501, 0x00},
	{0x5b00, 0x00},
	{0x5b01, 0x00},
	{0x5b02, 0x01},
	{0x5b03, 0xff},
	{0x5b04, 0x02},
	{0x5b05, 0x6c},
	{0x5b09, 0x02},
	{0x5e00, 0x00},
	{0x5e10, 0x1c},
	{0x0102, 0x01},
};

static struct regval_list sensor_2560x1440_30fps_regs[] = {
	{0x3611, 0x10},
	/* Lower VCO before reducing its divider: SCLK never exceeds 126 MHz. */
	{0x3614, 0x28},
	{REG_DLY, 5},
	{0x3612, 0x23},
	{REG_DLY, 5},
	{0x3613, 0x33},
	{0x370a, 0x24},
	{0x372a, 0x00},
	{0x372f, 0xa0},
	{0x3800, 0x03},
	{0x3801, 0x48},
	{0x3802, 0x03},
	{0x3803, 0x50},
	{0x3804, 0x0d},
	{0x3805, 0x57},
	{0x3806, 0x08},
	{0x3807, 0xff},
	{0x3808, 0x0a},
	{0x3809, 0x00},
	{0x380a, 0x05},
	{0x380b, 0xa0},
	{0x380c, 0x12},
	{0x380d, 0xc0},
	{0x380e, 0x0d},
	{0x380f, 0x05},
	{0x3810, 0x00},
	{0x3811, 0x04},
	{0x3812, 0x00},
	{0x3813, 0x04},
	{0x3814, 0x11},
	{0x3815, 0x11},
	{0x3820, 0x00},
	{0x3821, 0x04},
	{0x3836, 0x04},
	{0x3837, 0x01},
	{0x4601, 0x60},
	{0x4603, 0x01},
	{0x5401, 0x71},
	{0x5405, 0x80},
	{0x4020, 0x00},
	{0x4021, 0xe6},
	{0x4022, 0x09},
	{0x4023, 0x26},
	{0x4024, 0x0a},
	{0x4025, 0x08},
	{0x4026, 0x0a},
	{0x4027, 0x0e},
	{REG_DLY, 5},
};

static struct regval_list sensor_2560x1440_60fps_regs[] = {
	{0x3611, 0x10},
	/* Lower VCO before reducing its divider: SCLK never exceeds 126 MHz. */
	{0x3614, 0x28},
	{REG_DLY, 5},
	{0x3612, 0x23},
	{REG_DLY, 5},
	{0x3613, 0x33},
	{0x370a, 0x24},
	{0x372a, 0x00},
	{0x372f, 0xa0},
	{0x3800, 0x03},
	{0x3801, 0x48},
	{0x3802, 0x03},
	{0x3803, 0x50},
	{0x3804, 0x0d},
	{0x3805, 0x57},
	{0x3806, 0x08},
	{0x3807, 0xff},
	{0x3808, 0x0a},
	{0x3809, 0x00},
	{0x380a, 0x05},
	{0x380b, 0xa0},
	{0x380c, 0x12},
	{0x380d, 0xc0},
	{0x380e, 0x06},
	{0x380f, 0x83},
	{0x3810, 0x00},
	{0x3811, 0x04},
	{0x3812, 0x00},
	{0x3813, 0x04},
	{0x3814, 0x11},
	{0x3815, 0x11},
	{0x3820, 0x00},
	{0x3821, 0x04},
	{0x3836, 0x04},
	{0x3837, 0x01},
	{0x4601, 0x60},
	{0x4603, 0x01},
	{0x5401, 0x71},
	{0x5405, 0x80},
	{0x4020, 0x00},
	{0x4021, 0xe6},
	{0x4022, 0x09},
	{0x4023, 0x26},
	{0x4024, 0x0a},
	{0x4025, 0x08},
	{0x4026, 0x0a},
	{0x4027, 0x0e},
	{REG_DLY, 5},
};

/* Native 3840x2160 at 25 fps; V861 ISP requires its split-frame path. */
static struct regval_list sensor_3840x2160_25fps_regs[] = {
	{0x3612, 0x26},
	{0x370a, 0x24},
	{0x372a, 0x00},
	{0x372f, 0xa0},
	{0x3800, 0x00},
	{0x3801, 0xc8},
	{0x3802, 0x01},
	{0x3803, 0xe8},
	{0x3804, 0x0f},
	{0x3805, 0xd7},
	{0x3806, 0x0a},
	{0x3807, 0x67},
	{0x3808, 0x0f},
	{0x3809, 0x00},
	{0x380a, 0x08},
	{0x380b, 0x70},
	{0x380c, 0x12},
	{0x380d, 0xc0},
	{0x380e, 0x0b},
	{0x380f, 0x29},
	{0x3810, 0x00},
	{0x3811, 0x04},
	{0x3812, 0x00},
	{0x3813, 0x04},
	{0x3814, 0x11},
	{0x3815, 0x11},
	{0x3820, 0x00},
	{0x3821, 0x04},
	{0x3836, 0x04},
	{0x3837, 0x01},
	{0x4601, 0x60},
	{0x4603, 0x01},
	{0x4020, 0x00},
	{0x4021, 0xe6},
	{0x4022, 0x0e},
	{0x4023, 0x26},
	{0x4024, 0x0f},
	{0x4025, 0x08},
	{0x4026, 0x0f},
	{0x4027, 0x0e},
	{0x4603, 0x01},
	{0x5401, 0x71},
	{0x5405, 0x80},
	/* VCO 1200 MHz, timing clock 342857142 Hz; HTS 4800, VTS 2857. */
	{0x3614, 0x32},
};

/* Preserve the full binned analogue/black-level window; crop digitally. */

static struct regval_list sensor_1920x1080_30fps_regs[] = {
	{0x3611, 0x10},
	/* Lower VCO before reducing its divider: SCLK never exceeds 126 MHz. */
	{0x3614, 0x28},
	{REG_DLY, 5},
	{0x3612, 0x23},
	{REG_DLY, 5},
	{0x3613, 0x33},
	{0x370a, 0x26},
	{0x372a, 0x00},
	{0x372f, 0x90},
	{0x3800, 0x00},
	{0x3801, 0x08},
	{0x3802, 0x00},
	{0x3803, 0x04},
	{0x3804, 0x10},
	{0x3805, 0x97},
	{0x3806, 0x0c},
	{0x3807, 0x4b},
	{0x3808, 0x07},
	{0x3809, 0x80},
	{0x380a, 0x04},
	{0x380b, 0x38},
	{0x380c, 0x12},
	{0x380d, 0xc0},
	{0x380e, 0x0d},
	{0x380f, 0x05},
	{0x3810, 0x00},
	{0x3811, 0x64},
	{0x3812, 0x00},
	{0x3813, 0xf6},
	{0x3814, 0x31},
	{0x3815, 0x31},
	{0x3820, 0x02},
	{0x3821, 0x05},
	{0x3836, 0x08},
	{0x3837, 0x02},
	{0x4601, 0x60},
	{0x4603, 0x01},
	{0x5401, 0x61},
	{0x5405, 0x40},
	{0x4020, 0x00},
	{0x4021, 0xe4},
	{0x4022, 0x07},
	{0x4023, 0x5f},
	{0x4024, 0x08},
	{0x4025, 0x44},
	{0x4026, 0x08},
	{0x4027, 0x47},
	{REG_DLY, 5},
};

static struct regval_list sensor_1920x1080_60fps_regs[] = {
	{0x3611, 0x10},
	/* Lower VCO before reducing its divider: SCLK never exceeds 126 MHz. */
	{0x3614, 0x28},
	{REG_DLY, 5},
	{0x3612, 0x23},
	{REG_DLY, 5},
	{0x3613, 0x33},
	{0x370a, 0x26},
	{0x372a, 0x00},
	{0x372f, 0x90},
	{0x3800, 0x00},
	{0x3801, 0x08},
	{0x3802, 0x00},
	{0x3803, 0x04},
	{0x3804, 0x10},
	{0x3805, 0x97},
	{0x3806, 0x0c},
	{0x3807, 0x4b},
	{0x3808, 0x07},
	{0x3809, 0x80},
	{0x380a, 0x04},
	{0x380b, 0x38},
	{0x380c, 0x12},
	{0x380d, 0xc0},
	{0x380e, 0x06},
	{0x380f, 0x83},
	{0x3810, 0x00},
	{0x3811, 0x64},
	{0x3812, 0x00},
	{0x3813, 0xf6},
	{0x3814, 0x31},
	{0x3815, 0x31},
	{0x3820, 0x02},
	{0x3821, 0x05},
	{0x3836, 0x08},
	{0x3837, 0x02},
	{0x4601, 0x60},
	{0x4603, 0x01},
	{0x5401, 0x61},
	{0x5405, 0x40},
	{0x4020, 0x00},
	{0x4021, 0xe4},
	{0x4022, 0x07},
	{0x4023, 0x5f},
	{0x4024, 0x08},
	{0x4025, 0x44},
	{0x4026, 0x08},
	{0x4027, 0x47},
	{REG_DLY, 5},
};

static struct regval_list sensor_1280x720_30fps_regs[] = {
	{0x3611, 0x10},
	/* Lower VCO before reducing its divider: SCLK never exceeds 126 MHz. */
	{0x3614, 0x28},
	{REG_DLY, 5},
	{0x3612, 0x23},
	{REG_DLY, 5},
	{0x3613, 0x33},
	{0x370a, 0x26},
	{0x372a, 0x00},
	{0x372f, 0x90},
	{0x3800, 0x00},
	{0x3801, 0x08},
	{0x3802, 0x00},
	{0x3803, 0x04},
	{0x3804, 0x10},
	{0x3805, 0x97},
	{0x3806, 0x0c},
	{0x3807, 0x4b},
	{0x3808, 0x05},
	{0x3809, 0x00},
	{0x380a, 0x02},
	{0x380b, 0xd0},
	{0x380c, 0x12},
	{0x380d, 0xc0},
	{0x380e, 0x0d},
	{0x380f, 0x05},
	{0x3810, 0x01},
	{0x3811, 0xa4},
	{0x3812, 0x01},
	{0x3813, 0xaa},
	{0x3814, 0x31},
	{0x3815, 0x31},
	{0x3820, 0x02},
	{0x3821, 0x05},
	{0x3836, 0x08},
	{0x3837, 0x02},
	{0x4601, 0x60},
	/* Automatic FIFO start is required for this narrow digital crop. */
	{0x4603, 0x00},
	{0x5401, 0x61},
	{0x5405, 0x40},
	{0x4020, 0x00},
	{0x4021, 0xe4},
	{0x4022, 0x07},
	{0x4023, 0x5f},
	{0x4024, 0x08},
	{0x4025, 0x44},
	{0x4026, 0x08},
	{0x4027, 0x47},
	{REG_DLY, 5},
};

static struct regval_list sensor_1280x720_60fps_regs[] = {
	{0x3611, 0x10},
	/* Lower VCO before reducing its divider: SCLK never exceeds 126 MHz. */
	{0x3614, 0x28},
	{REG_DLY, 5},
	{0x3612, 0x23},
	{REG_DLY, 5},
	{0x3613, 0x33},
	{0x370a, 0x26},
	{0x372a, 0x00},
	{0x372f, 0x90},
	{0x3800, 0x00},
	{0x3801, 0x08},
	{0x3802, 0x00},
	{0x3803, 0x04},
	{0x3804, 0x10},
	{0x3805, 0x97},
	{0x3806, 0x0c},
	{0x3807, 0x4b},
	{0x3808, 0x05},
	{0x3809, 0x00},
	{0x380a, 0x02},
	{0x380b, 0xd0},
	{0x380c, 0x12},
	{0x380d, 0xc0},
	{0x380e, 0x06},
	{0x380f, 0x83},
	{0x3810, 0x01},
	{0x3811, 0xa4},
	{0x3812, 0x01},
	{0x3813, 0xaa},
	{0x3814, 0x31},
	{0x3815, 0x31},
	{0x3820, 0x02},
	{0x3821, 0x05},
	{0x3836, 0x08},
	{0x3837, 0x02},
	{0x4601, 0x60},
	/* Automatic FIFO start is required for this narrow digital crop. */
	{0x4603, 0x00},
	{0x5401, 0x61},
	{0x5405, 0x40},
	{0x4020, 0x00},
	{0x4021, 0xe4},
	{0x4022, 0x07},
	{0x4023, 0x5f},
	{0x4024, 0x08},
	{0x4025, 0x44},
	{0x4026, 0x08},
	{0x4027, 0x47},
	{REG_DLY, 5},
};

static struct regval_list sensor_fmt_raw[] = {

};

static int sensor_g_exp(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);

	*value = info->exp;
	sensor_dbg("sensor_get_exposure = %d\n", info->exp);
	return 0;
}

static int sensor_s_exp(struct v4l2_subdev *sd, unsigned int exp_val)
{
	unsigned char explow, expmid, exphigh;
	int ret;

	if (ov13850_sensor_vts > 16)
		exp_val = clamp_t(unsigned int, exp_val, 16, (ov13850_sensor_vts - 16) << 4);
	if (exp_val > 0xfffff)
		exp_val = 0xfffff;

	exphigh = (unsigned char)((exp_val >> 16) & 0x0f);
	expmid = (unsigned char)((exp_val >> 8) & 0xff);
	explow = (unsigned char)((exp_val >> 0)& 0xff);

	ret = sensor_write(sd, 0x3502, explow);
	if (ret < 0)
		return ret;
	ret = sensor_write(sd, 0x3501, expmid);
	if (ret < 0)
		return ret;
	ret = sensor_write(sd, 0x3500, exphigh);
	if (ret < 0)
		return ret;

	to_state(sd)->exp = exp_val;
	return 0;
}

static int sensor_g_gain(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);

	*value = info->gain;
	sensor_dbg("sensor_get_gain = %d\n", info->gain);
	return 0;
}

static int sensor_s_gain(struct v4l2_subdev *sd, int gain_val)
{
	unsigned char Againlow = 0;
	unsigned char Againhigh = 0;
	unsigned char Dgainlow = 0;
	unsigned char Dgainhigh = 0;
	unsigned int Dgain = 0;
	int ret;

	if (gain_val < 1 * 16)
		gain_val = 16;
	if (gain_val > 64 * 16 - 1)
		gain_val = 64 * 16 - 1;

	if (gain_val <= 15*16) { /* Tgain = Again*Dgain Dgain = 1 */
		Againhigh = 0x00;
		Againlow = (unsigned char)(gain_val & 0xff);
		Dgainhigh = 0x01;
		Dgainlow = 0x00;
	} else { /* Tgain = Again*Dgain  Again = 15 */
		Againhigh = 0x00;
		Againlow = 0xf0;
		Dgain = ((gain_val/15)<<4); /* Dgain 1x = 256 */
		Dgainhigh = Dgain>>8;
		Dgainlow = Dgain&0xff;
	}
	ret = sensor_write(sd, 0x350a, Againhigh);
	if (ret < 0)
		return ret;
	ret = sensor_write(sd, 0x350b, Againlow);
	if (ret < 0)
		return ret;
	ret = sensor_write(sd, 0x5502, Dgainhigh);
	if (ret < 0)
		return ret;
	ret = sensor_write(sd, 0x5503, Dgainlow);
	if (ret < 0)
		return ret;

	sensor_dbg("sensor_set_gain = %d\n", gain_val);

	to_state(sd)->gain = gain_val;
	return 0;
}

static int sensor_s_exp_gain(struct v4l2_subdev *sd,
			     struct sensor_exp_gain *exp_gain)
{
	int exp_val, gain_val, ret;
	struct sensor_info *info = to_state(sd);

	exp_val = exp_gain->exp_val;
	gain_val = exp_gain->gain_val;
	/* Fixed VTS: AE must not lower the selected frame rate. */
	exp_val = clamp(exp_val, 16, (ov13850_sensor_vts - 16) << 4);
	gain_val = clamp(gain_val, 16, 64 * 16 - 1);


	ret = sensor_write(sd, 0x3208, 0x00); /* begin group */
	if (ret < 0)
		return ret;
	ret = sensor_write(sd, 0x380f, ov13850_sensor_vts & 0xff);
	if (ret < 0)
		goto end_group;
	ret = sensor_write(sd, 0x380e, ov13850_sensor_vts >> 8);
	if (ret < 0)
		goto end_group;
	ret = sensor_s_exp(sd, exp_val);
	if (ret < 0)
		goto end_group;
	ret = sensor_s_gain(sd, gain_val);
end_group:
	{
		int end_ret = sensor_write(sd, 0x3208, 0x10);

		if (ret < 0)
			return ret;
		if (end_ret < 0)
			return end_ret;
	}
	ret = sensor_write(sd, 0x3208, 0xa0); /* launch complete group */
	if (ret < 0)
		return ret;
	info->exp = exp_val;
	info->gain = gain_val;
	return 0;
}

static int sensor_s_sw_stby(struct v4l2_subdev *sd, int on_off)
{
	int ret;
	data_type rdval;

	ret = sensor_read(sd, 0x0100, &rdval);
	if (ret != 0)
		return ret;

	if (on_off == CSI_GPIO_LOW)
		ret = sensor_write(sd, 0x0100, rdval & 0xfe);
	else
		ret = sensor_write(sd, 0x0100, rdval | 0x01);

	return ret;
}

/*
 * Stuff that knows about the sensor.
 */

static int sensor_power(struct v4l2_subdev *sd, int on)
{
	int ret = 0;

	switch (on) {
	case STBY_ON:
		sensor_dbg("STBY_ON!\n");
		ret = sensor_s_sw_stby(sd, CSI_GPIO_LOW);
		if (ret < 0)
			sensor_err("soft stby falied!\n");
		usleep_range(10000, 12000);
		cci_lock(sd);
		vin_gpio_write(sd, PWDN, CSI_GPIO_LOW);
		cci_unlock(sd);
		vin_set_mclk(sd, OFF);
		break;
	case STBY_OFF:
		sensor_dbg("STBY_OFF!\n");
		cci_lock(sd);
		vin_set_mclk_freq(sd, MCLK);
		vin_set_mclk(sd, ON);
		usleep_range(10000, 12000);
		vin_gpio_write(sd, PWDN, CSI_GPIO_HIGH);
		usleep_range(10000, 12000);
		ret = sensor_s_sw_stby(sd, CSI_GPIO_HIGH);
		if (ret < 0)
			sensor_err("soft stby off falied!\n");
		cci_unlock(sd);
		break;
	case PWR_ON:
		sensor_dbg("PWR_ON!\n");
		cci_lock(sd);
		vin_gpio_set_status(sd, PWDN, 1);
		vin_gpio_set_status(sd, RESET, 1);
		vin_gpio_write(sd, PWDN, CSI_GPIO_LOW);
		vin_gpio_write(sd, RESET, CSI_GPIO_LOW);
		vin_set_pmu_channel(sd, AFVDD, ON);
		vin_set_pmu_channel(sd, IOVDD, ON);
		usleep_range(5000, 6000);
		vin_gpio_write(sd, PWDN, CSI_GPIO_HIGH);
		vin_set_pmu_channel(sd, CAMERAVDD, ON);
		vin_set_pmu_channel(sd, AVDD, ON);
		usleep_range(5000, 6000);
		vin_set_pmu_channel(sd, DVDD, ON);
		usleep_range(5000, 6000);
		vin_gpio_write(sd, RESET, CSI_GPIO_HIGH);
		usleep_range(5000, 6000);
		vin_set_mclk_freq(sd, MCLK);
		vin_set_mclk(sd, ON);
		usleep_range(5000, 6000);
		cci_unlock(sd);
		break;
	case PWR_OFF:
		sensor_dbg("PWR_OFF!\n");
		cci_lock(sd);
		vin_set_mclk(sd, OFF);
		usleep_range(5000, 6000);
		vin_gpio_write(sd, RESET, CSI_GPIO_LOW);
		usleep_range(5000, 6000);
		vin_set_pmu_channel(sd, DVDD, OFF);
		usleep_range(5000, 6000);
		vin_set_pmu_channel(sd, AVDD, OFF);
		vin_set_pmu_channel(sd, CAMERAVDD, OFF);
		usleep_range(5000, 6000);
		vin_set_pmu_channel(sd, IOVDD, OFF);
		vin_set_pmu_channel(sd, AFVDD, OFF);
		vin_gpio_write(sd, PWDN, CSI_GPIO_LOW);
		usleep_range(5000, 6000);
		vin_gpio_set_status(sd, RESET, 0);
		vin_gpio_set_status(sd, PWDN, 0);
		cci_unlock(sd);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int sensor_reset(struct v4l2_subdev *sd, u32 val)
{
	switch (val) {
	case 0:
		vin_gpio_write(sd, RESET, CSI_GPIO_HIGH);
		usleep_range(100, 120);
		break;
	case 1:
		vin_gpio_write(sd, RESET, CSI_GPIO_LOW);
		usleep_range(100, 120);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int sensor_detect(struct v4l2_subdev *sd)
{
	data_type high = 0, low = 0;
	int ret = -ENODEV, attempt;

	for (attempt = 0; attempt < 3; attempt++) {
		ret = sensor_read(sd, 0x300a, &high);
		if (!ret)
			ret = sensor_read(sd, 0x300b, &low);
		if (!ret && high == 0xd8 && low == 0x50)
			return 0;
		if (!ret)
			ret = -ENODEV;
		usleep_range(5000, 5500);
	}
	return ret;
}

static int sensor_init(struct v4l2_subdev *sd, u32 val)
{
	int ret;
	struct sensor_info *info = to_state(sd);

	sensor_dbg("sensor_init\n");

	/*Make sure it is a target sensor */
	ret = sensor_detect(sd);
	if (ret) {
		sensor_err("chip found is not an target chip.\n");
		return ret;
	}

	info->focus_status = 0;
	info->low_speed = 0;
	info->width = 2560;
	info->height = 1440;
	info->hflip = 0;
	info->vflip = 0;

	info->tpf.numerator = 1;
	info->tpf.denominator = SENSOR_FRAME_RATE;

	info->preview_first_flag = 1;

	return 0;
}

static int sensor_g_flip(struct v4l2_subdev *sd, struct sensor_flip *flip)
{
	data_type vertical, horizontal;
	int ret;

	if (!flip)
		return -EINVAL;
	ret = sensor_read(sd, 0x3820, &vertical);
	if (ret < 0)
		return ret;
	ret = sensor_read(sd, 0x3821, &horizontal);
	if (ret < 0)
		return ret;
	/* OV13850 datasheet table 4-1: bit 2 in both timing registers.
	 * Report physical readout orientation; do not rewrite the registers,
	 * info->hflip/vflip, or the MEDIA_BUS_FMT_SBGGR10_1X10 declaration. */
	flip->hflip = !!(horizontal & 0x04);
	flip->vflip = !!(vertical & 0x04);
	return 0;
}

static long sensor_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret = 0;
	struct sensor_info *info = to_state(sd);

	switch (cmd) {
	case GET_CURRENT_WIN_CFG:
		if (info->current_wins != NULL) {
			memcpy(arg, info->current_wins,
			       sizeof(struct sensor_win_size));
			ret = 0;
		} else {
			sensor_err("empty wins!\n");
			ret = -1;
		}
		break;
	case VIDIOC_VIN_SENSOR_GET_FPS:
		if (!arg || !info->current_wins)
			return -EINVAL;
		((struct sensor_fps *)arg)->fps = info->current_wins->fps_fixed;
		break;
	case VIDIOC_VIN_SENSOR_GET_FLIP:
		ret = sensor_g_flip(sd, (struct sensor_flip *)arg);
		break;
	case SET_FPS:
		ret = 0;
		break;
	case VIDIOC_VIN_SENSOR_EXP_GAIN:
		ret = sensor_s_exp_gain(sd, (struct sensor_exp_gain *)arg);
		break;
	case VIDIOC_VIN_SENSOR_CFG_REQ:
		sensor_cfg_req(sd, (struct sensor_config *)arg);
		break;
	case VIDIOC_VIN_ACT_INIT:
		ret = actuator_init(sd, (struct actuator_para *)arg);
		break;
	case VIDIOC_VIN_ACT_SET_CODE:
		ret = actuator_set_code(sd, (struct actuator_ctrl *)arg);
		break;
	case VIDIOC_VIN_FLASH_EN:
		ret = flash_en(sd, (struct flash_para *)arg);
		break;
	default:
		return -EINVAL;
	}
	return ret;
}

/*
 * Store information about the video data format.
 */
static struct sensor_format_struct sensor_formats[] = {
	{
		.desc = "Raw RGB Bayer",
		.mbus_code = MEDIA_BUS_FMT_SBGGR10_1X10,
		.regs = sensor_fmt_raw,
		.regs_size = ARRAY_SIZE(sensor_fmt_raw),
		.bpp = 1
	},
};
#define N_FMTS ARRAY_SIZE(sensor_formats)

/*
 * Then there is the issue of window sizes.  Try to capture the info here.
 */

static struct sensor_win_size sensor_win_sizes[] = {
	{
		.width = 2560, .height = 1440,
		.hts = 4800, .vts = 3333, .pclk = 480000000,
		.mipi_bps = 1200000000, .fps_fixed = 30, .bin_factor = 1,
		.intg_min = 16, .intg_max = (3333 - 16) << 4,
		.gain_min = 16, .gain_max = 15 << 4,
		.regs = sensor_2560x1440_30fps_regs, .regs_size = ARRAY_SIZE(sensor_2560x1440_30fps_regs),
	},
	{
		.width = 2560, .height = 1440,
		.hts = 4800, .vts = 1667, .pclk = 480000000,
		.mipi_bps = 1200000000, .fps_fixed = 60, .bin_factor = 1,
		.intg_min = 16, .intg_max = (1667 - 16) << 4,
		.gain_min = 16, .gain_max = 15 << 4,
		.regs = sensor_2560x1440_60fps_regs, .regs_size = ARRAY_SIZE(sensor_2560x1440_60fps_regs),
	},
	{
		.width = 3840, .height = 2160,
		.hts = 4800, .vts = 2857, .pclk = 342857142,
		.mipi_bps = 1200000000, .fps_fixed = 25, .bin_factor = 1,
		.intg_min = 16, .intg_max = (2857 - 16) << 4,
		.gain_min = 16, .gain_max = 15 << 4,
		.regs = sensor_3840x2160_25fps_regs, .regs_size = ARRAY_SIZE(sensor_3840x2160_25fps_regs),
	},
	{
		.width = 1920, .height = 1080,
		.hts = 4800, .vts = 3333, .pclk = 480000000,
		.mipi_bps = 1200000000, .fps_fixed = 30, .bin_factor = 2,
		.intg_min = 16, .intg_max = (3333 - 16) << 4,
		.gain_min = 16, .gain_max = 15 << 4,
		.regs = sensor_1920x1080_30fps_regs, .regs_size = ARRAY_SIZE(sensor_1920x1080_30fps_regs),
	},
	{
		.width = 1920, .height = 1080,
		.hts = 4800, .vts = 1667, .pclk = 480000000,
		.mipi_bps = 1200000000, .fps_fixed = 60, .bin_factor = 2,
		.intg_min = 16, .intg_max = (1667 - 16) << 4,
		.gain_min = 16, .gain_max = 15 << 4,
		.regs = sensor_1920x1080_60fps_regs, .regs_size = ARRAY_SIZE(sensor_1920x1080_60fps_regs),
	},
	{
		.width = 1280, .height = 720,
		.hts = 4800, .vts = 3333, .pclk = 480000000,
		.mipi_bps = 1200000000, .fps_fixed = 30, .bin_factor = 2,
		.intg_min = 16, .intg_max = (3333 - 16) << 4,
		.gain_min = 16, .gain_max = 15 << 4,
		.regs = sensor_1280x720_30fps_regs, .regs_size = ARRAY_SIZE(sensor_1280x720_30fps_regs),
	},
	{
		.width = 1280, .height = 720,
		.hts = 4800, .vts = 1667, .pclk = 480000000,
		.mipi_bps = 1200000000, .fps_fixed = 60, .bin_factor = 2,
		.intg_min = 16, .intg_max = (1667 - 16) << 4,
		.gain_min = 16, .gain_max = 15 << 4,
		.regs = sensor_1280x720_60fps_regs, .regs_size = ARRAY_SIZE(sensor_1280x720_60fps_regs),
	},
};

#define N_WIN_SIZES (ARRAY_SIZE(sensor_win_sizes))

static int sensor_g_mbus_config(struct v4l2_subdev *sd, unsigned int pad,
				struct v4l2_mbus_config *cfg)
{
	cfg->type = V4L2_MBUS_CSI2_DPHY;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
	cfg->bus.mipi_csi2.num_data_lanes = 0 | V4L2_MBUS_CSI2_4_LANE | V4L2_MBUS_CSI2_CHANNEL_0;
#else
	cfg->flags = 0 | V4L2_MBUS_CSI2_4_LANE | V4L2_MBUS_CSI2_CHANNEL_0;
#endif

	return 0;
}

static int sensor_g_ctrl(struct v4l2_ctrl *ctrl)
{
	struct sensor_info *info =
			container_of(ctrl->handler,
			struct sensor_info, handler);
	struct v4l2_subdev *sd = &info->sd;

	switch (ctrl->id) {
	case V4L2_CID_GAIN:
		return sensor_g_gain(sd, &ctrl->val);
	case V4L2_CID_EXPOSURE:
		return sensor_g_exp(sd, &ctrl->val);
	}
	return -EINVAL;
}

static int sensor_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct sensor_info *info =
			container_of(ctrl->handler,
			struct sensor_info, handler);
	struct v4l2_subdev *sd = &info->sd;

	switch (ctrl->id) {
	case V4L2_CID_GAIN:
		return sensor_s_gain(sd, ctrl->val);
	case V4L2_CID_EXPOSURE:
		return sensor_s_exp(sd, ctrl->val);
	}
	return -EINVAL;
}

static int ov13850_write_regs(struct v4l2_subdev *sd,
			      struct regval_list *regs, int count)
{
	int i, ret;

	for (i = 0; i < count; i++) {
		if (regs[i].addr == REG_DLY) {
			usleep_range(regs[i].data * 1000,
				     regs[i].data * 1000 + 100);
			continue;
		}
		ret = sensor_write(sd, regs[i].addr, regs[i].data);
		if (ret < 0) {
			sensor_err("register write failed: 0x%04x=0x%02x (%d)\n",
				   regs[i].addr, regs[i].data, ret);
			return ret;
		}
	}
	return 0;
}

static int sensor_reg_init(struct sensor_info *info)
{
	struct v4l2_subdev *sd = &info->sd;
	struct sensor_win_size *wsize = info->current_wins;
	struct sensor_exp_gain exp_gain;
	int ret;

	if (!wsize)
		return -EINVAL;
	ret = ov13850_write_regs(sd, sensor_default_regs,
				 ARRAY_SIZE(sensor_default_regs));
	if (ret < 0)
		return ret;
	ret = ov13850_write_regs(sd, wsize->regs, wsize->regs_size);
	if (ret < 0)
		return ret;

	info->width = wsize->width;
	info->height = wsize->height;
	ov13850_sensor_vts = wsize->vts;
	exp_gain.exp_val = info->exp ? info->exp : 20000;
	exp_gain.gain_val = info->gain ? info->gain : 16;
	ret = sensor_s_exp_gain(sd, &exp_gain);
	if (ret < 0)
		return ret;
	return sensor_write(sd, 0x0100, 0x01);
}

static int sensor_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct sensor_info *info = to_state(sd);

	sensor_print("%s on = %d, %d*%d fps: %d code: %x\n", __func__, enable,
			info->current_wins->width, info->current_wins->height,
			info->current_wins->fps_fixed, info->fmt->mbus_code);

	if (!enable)
		return sensor_write(sd, 0x0100, 0x00);

	return sensor_reg_init(info);
}

/* ----------------------------------------------------------------------- */

static const struct v4l2_ctrl_ops sensor_ctrl_ops = {
	.g_volatile_ctrl = sensor_g_ctrl,
	.s_ctrl = sensor_s_ctrl,
};

static const struct v4l2_subdev_core_ops sensor_core_ops = {
	.reset = sensor_reset,
	.init = sensor_init,
	.s_power = sensor_power,
	.ioctl = sensor_ioctl,
#if IS_ENABLED(CONFIG_COMPAT)
	.compat_ioctl32 = sensor_compat_ioctl32,
#endif
};

static const struct v4l2_subdev_video_ops sensor_video_ops = {
	.s_stream = sensor_s_stream,
};

static const struct v4l2_subdev_pad_ops sensor_pad_ops = {
	.enum_mbus_code = sensor_enum_mbus_code,
	.enum_frame_size = sensor_enum_frame_size,
	.enum_frame_interval = sensor_enum_frame_interval,
	.get_fmt = sensor_get_fmt,
	.set_fmt = sensor_set_fmt,
	.get_mbus_config = sensor_g_mbus_config,
};

static const struct v4l2_subdev_ops sensor_ops = {
	.core = &sensor_core_ops,
	.video = &sensor_video_ops,
	.pad = &sensor_pad_ops,
};

/* ----------------------------------------------------------------------- */
static struct cci_driver cci_drv = {
	.name = SENSOR_NAME,
	.addr_width = CCI_BITS_16,
	.data_width = CCI_BITS_8,
};

static int sensor_init_controls(struct v4l2_subdev *sd,
			const struct v4l2_ctrl_ops *ops)
{
	struct sensor_info *info = to_state(sd);
	struct v4l2_ctrl_handler *handler = &info->handler;
	struct v4l2_ctrl *ctrl;
	int ret = 0;

	v4l2_ctrl_handler_init(handler, 2);

	v4l2_ctrl_new_std(handler, ops, V4L2_CID_GAIN, 16,
				64 * 16 - 1, 1, 16);
	ctrl = v4l2_ctrl_new_std(handler, ops, V4L2_CID_EXPOSURE, 16,
				(3333 - 16) << 4, 1, 16);
	if (ctrl != NULL)
		ctrl->flags |= V4L2_CTRL_FLAG_VOLATILE;

	if (handler->error) {
		ret = handler->error;
		v4l2_ctrl_handler_free(handler);
	}

	sd->ctrl_handler = handler;

	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
static int sensor_probe(struct i2c_client *client)
#else
static int sensor_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
#endif
{
	struct v4l2_subdev *sd;
	struct sensor_info *info;

	info = kzalloc(sizeof(struct sensor_info), GFP_KERNEL);
	if (info == NULL)
		return -ENOMEM;
	mutex_init(&info->lock);
	sd = &info->sd;
	cci_dev_probe_helper(sd, client, &sensor_ops, &cci_drv);
	sensor_init_controls(sd, &sensor_ctrl_ops);

	info->fmt = &sensor_formats[0];
	info->fmt_pt = &sensor_formats[0];
	info->win_pt = &sensor_win_sizes[0];
	info->fmt_num = N_FMTS;
	info->win_size_num = N_WIN_SIZES;
	info->sensor_field = V4L2_FIELD_NONE;
	info->af_first_flag = 1;
	info->time_hs = 0x20;
	info->deskew = 0x02;
	info->exp = 0;
	info->gain = 0;

	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0)
static int sensor_remove(struct i2c_client *client)
#else
static void sensor_remove(struct i2c_client *client)
#endif
{
	struct v4l2_subdev *sd;

	sd = cci_dev_remove_helper(client, &cci_drv);
	kfree(to_state(sd));
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0)
	return 0;
#endif
}

static const struct i2c_device_id sensor_id[] = {
	{SENSOR_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, sensor_id);

static struct i2c_driver sensor_driver = {
	.driver = {
		   .owner = THIS_MODULE,
		   .name = SENSOR_NAME,
		   },
	.probe = sensor_probe,
	.remove = sensor_remove,
	.id_table = sensor_id,
};
static __init int init_sensor(void)
{
	return cci_dev_init_helper(&sensor_driver);
}

static __exit void exit_sensor(void)
{
	cci_dev_exit_helper(&sensor_driver);
}

module_init(init_sensor);
module_exit(exit_sensor);
