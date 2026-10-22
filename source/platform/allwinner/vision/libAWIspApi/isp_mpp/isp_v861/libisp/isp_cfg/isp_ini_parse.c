
/*
 ******************************************************************************
 *
 * isp_ini_parse.c
 *
 * Hawkview ISP - isp_ini_parse.c module
 *
 * Copyright (c) 2015 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2015/11/23	ISP Tuning Tools Support
 *
 ******************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../iniparser/src/iniparser.h"
#include "../include/isp_ini_parse.h"
#include "../include/isp_manage.h"
#include "../include/isp_debug.h"
#include "../isp_dev/tools.h"
#include "bsdiff/isp_bsdiff.h"

#if (ISP_VERSION == 610)
#if defined(SENSOR_CV2003) || defined(SENSOR_SC231HAI) || defined(SENSOR_SC235HAI) ||  defined(SENSOR_GC2083) || \
	defined(SENSOR_GC4663) || defined(SENSOR_GC8613) || defined(SENSOR_SC485SL) || defined(SENSOR_SC635HAI) ||  \
	defined(SENSOR_SC2337P) || defined(SENSOR_CV8002) || defined(SENSOR_SC533HAI) || defined(SENSOR_CV2005) ||  \
	defined(SENSOR_IMX681) || defined(SENSOR_IMX766) || defined(SENSOR_CV5003) || defined(SENSOR_GC13A2) || defined(SENSOR_OV13850)

#ifdef SENSOR_CV2003
#include "SENSOR_H/cv2003/cv2003_mipi_isp610_20260109_103158_day.h"
#include "SENSOR_H/cv2003/cv2003_mipi_isp610_20260109_103158_ir.h"
#endif

#ifdef SENSOR_SC231HAI
#include "SENSOR_H/sc231hai/sc231hai_mipi_default_ini_isp610.h"
#endif

#ifdef SENSOR_SC2337P
#include "SENSOR_H/sc2337p/sc2337p_mipi_default_ini_isp610.h"
#include "SENSOR_H/sc2337p/sc2337p_mipi_default_ini_ir_isp610.h"
#include "SENSOR_H/sc2337p/sc2337p_mipi_isp610_20251218_170620_base_aiisp.h"
#endif

#ifdef SENSOR_SC235HAI
#include "SENSOR_H/sc235hai/sc235hai_mipi_default_ini_isp610.h"
#include "SENSOR_H/sc235hai/sc235hai_mipi_isp610_20260410_103408_day.h"
#include "SENSOR_H/sc235hai/sc235hai_mipi_isp610_20260109_204129_ir.h"
#include "SENSOR_H/sc235hai/sc235hai_mipi_isp610_20260416_211412_aiisp.h"
#endif

#ifdef SENSOR_OV13850
#include "SENSOR_H/ov13850/ov13850_mipi_isp610_20261021_195907.h"
#endif

#ifdef SENSOR_GC2083
#include "SENSOR_H/gc2083/gc2083_mipi_default_ini_isp610.h"
#endif

#ifdef SENSOR_GC8613
#include "SENSOR_H/gc8613/gc8613_mipi_isp610_20260109_211717_day.h"
#include "SENSOR_H/gc8613/gc8613_mipi_isp610_20260109_213204_ir.h"
#endif

#ifdef SENSOR_SC485SL
#include "SENSOR_H/sc485sl/sc485sl_mipi_isp610_20260109_172438_day.h"
#include "SENSOR_H/sc485sl/sc485sl_mipi_isp610_20260109_174233_ir.h"
#include "SENSOR_H/sc485sl/sc485sl_mipi_isp610_20251024_192746_aiisp.h"
#endif

#ifdef SENSOR_GC4663
#include "SENSOR_H/gc4663/gc4663_mipi_default_ini_isp610.h"
#endif

#ifdef SENSOR_SC635HAI
#include "SENSOR_H/sc635hai/sc635hai_mipi_default_int_isp610.h"
#endif

#ifdef SENSOR_CV8002
#include "SENSOR_H/cv8002/cv8002_mipi_isp610_20251226_202226.h"
#include "SENSOR_H/cv8002/cv8002_mipi_isp610_20251014_202353_ir.h"
#endif

#ifdef SENSOR_SC533HAI
#include "SENSOR_H/sc533hai/sc533hai_mipi_isp610_20260116_092500_v12.h"
#include "SENSOR_H/sc533hai/sc533hai_mipi_isp610_20260512_212640_v14_wdr.h"
#endif

#ifdef SENSOR_CV2005
#include "SENSOR_H/cv2005/cv2005_mipi_isp610_20251202_170130.h"
#endif

#ifdef SENSOR_CV5003
#include "SENSOR_H/cv5003/cv5003_mipi_isp610_20260417_145901_fullsize.h"
#include "SENSOR_H/cv5003/cv5003_mipi_isp610_20260515_151917_eis.h"
#endif

#ifdef SENSOR_IMX681
#include "SENSOR_H/imx681/imx681_mipi_isp610_20251023_150150uvcp.h"
#endif

#ifdef SENSOR_IMX766
#include "SENSOR_H/imx766/imx766_mipi_default_pdaf.h"
#endif

#ifdef SENSOR_GC13A2
#include "SENSOR_H/gc13a2/gc13a2_mipi_default_ini_isp610_pdaf.h"
#endif

#else
#include "SENSOR_H/gc4663/gc4663_mipi_default_ini_isp610.h"
#endif
#else
#include "SENSOR_H/gc4663/gc4663_mipi_default_ini_isp610.h"
#endif

#define ISP_LIB_USE_INIPARSER 1
#define EXTERNEL_BASE_PATH_COLOR "/tmp/isp_test/color/"
#define EXTERNEL_BASE_PATH_IR    "/tmp/isp_test/ir/"
#define EXTERNEL_BASE_PATH_AIISP "/tmp/isp_test/aiisp/"

unsigned int isp_cfg_log_param = ISP_LOG_CFG;

#define SIZE_OF_LSC_TBL     (12*768*2)
#define SIZE_OF_GAMMA_TBL   (5*1024*3*2)

#if ISP_LIB_USE_INIPARSER

#define SET_ISP_SINGLE_VALUE(struct_name, key) \
void set_##key(struct isp_param_config *isp_ini_cfg, void *value, int len) \
{\
	isp_ini_cfg->struct_name.key = *(int *)value;\
}

#define SET_ISP_ARRAY_VALUE(struct_name, key) \
void set_##key(struct isp_param_config *isp_ini_cfg, void *value, int len) \
{\
	int i, *tmp = (int *)value;\
	for (i = 0; i < len; i++) \
		isp_ini_cfg->struct_name.key[i] = tmp[i];\
}

#define SET_ISP_ARRAY_INT(struct_name, key) \
void set_##key(struct isp_param_config *isp_ini_cfg, void *value, int len)\
{\
	memcpy(&isp_ini_cfg->struct_name.key[0], value, 4*len);\
}

#define SET_ISP_STRUCT_CHAR(struct_name, key) \
void set_##key(struct isp_param_config *isp_ini_cfg, void *value, int len)\
{\
	int i, *src = (int *)value;\
	HW_U8 *dst = (HW_U8 *)&isp_ini_cfg->struct_name.key;\
	for (i = 0; i < len; i++) {\
		dst[i] = src[i];\
	}\
}
#define SET_ISP_DYNAMIC_SINGLE_VALUE(struct_name, key, idx) \
void set_##key##_##idx(struct isp_param_config *isp_ini_cfg, void *value, int len)\
{\
	isp_ini_cfg->struct_name.isp_dynamic_cfg[idx].key = *(int *)value;\
}

#define SET_ISP_DYNAMIC_ARRAY_SHORT(struct_name, key, idx) \
void set_##key##_##idx(struct isp_param_config *isp_ini_cfg, void *value, int len)\
{\
	int i, *src = (int *)value;\
	for (i = 0; i < len; i++) {\
		isp_ini_cfg->struct_name.isp_dynamic_cfg[idx].key[i] = src[i];\
	}\
}

#define SET_ISP_CM_VALUE(key, idx) \
void set_##key##idx(struct isp_param_config *isp_ini_cfg, void *value, int len)\
{\
	int *tmp = (int *)value;\
	struct isp_rgb2rgb_gain_offset *color_matrix = &isp_ini_cfg->isp_tunning_settings.color_matrix_ini[idx];\
	color_matrix->matrix[0][0] = tmp[0];\
	color_matrix->matrix[0][1] = tmp[1];\
	color_matrix->matrix[0][2] = tmp[2];\
	color_matrix->matrix[1][0] = tmp[3];\
	color_matrix->matrix[1][1] = tmp[4];\
	color_matrix->matrix[1][2] = tmp[5];\
	color_matrix->matrix[2][0] = tmp[6];\
	color_matrix->matrix[2][1] = tmp[7];\
	color_matrix->matrix[2][2] = tmp[8];\
	color_matrix->offset[0] = tmp[9];\
	color_matrix->offset[1] = tmp[10];\
	color_matrix->offset[2] = tmp[11];\
}

#define ISP_FILE_SINGLE_ATTR(main_key, sub_key)\
{\
	#main_key,  #sub_key, 1,  set_##sub_key,\
}

#define ISP_FILE_ARRAY_ATTR(main_key, sub_key , len)\
{\
	#main_key, #sub_key, len,  set_##sub_key,\
}

#define ISP_FILE_STRUCT_ARRAY_ATTR(main_key, sub_key , len)\
{\
	#main_key, #sub_key, len,  set_##main_key,\
}

SET_ISP_SINGLE_VALUE(isp_test_settings, isp_test_mode);
SET_ISP_SINGLE_VALUE(isp_test_settings, isp_test_exptime);
SET_ISP_SINGLE_VALUE(isp_test_settings, exp_line_start);
SET_ISP_SINGLE_VALUE(isp_test_settings, exp_line_step);
SET_ISP_SINGLE_VALUE(isp_test_settings, exp_line_end);
SET_ISP_SINGLE_VALUE(isp_test_settings, exp_change_interval);
SET_ISP_SINGLE_VALUE(isp_test_settings, isp_test_gain);
SET_ISP_SINGLE_VALUE(isp_test_settings, gain_start);
SET_ISP_SINGLE_VALUE(isp_test_settings, gain_step);
SET_ISP_SINGLE_VALUE(isp_test_settings, gain_end);
SET_ISP_SINGLE_VALUE(isp_test_settings, gain_change_interval);

SET_ISP_SINGLE_VALUE(isp_test_settings, isp_test_focus);
SET_ISP_SINGLE_VALUE(isp_test_settings, focus_start);
SET_ISP_SINGLE_VALUE(isp_test_settings, focus_step);
SET_ISP_SINGLE_VALUE(isp_test_settings, focus_end);
SET_ISP_SINGLE_VALUE(isp_test_settings, focus_change_interval);
SET_ISP_SINGLE_VALUE(isp_test_settings, isp_log_param);
SET_ISP_SINGLE_VALUE(isp_test_settings, isp_gain);
SET_ISP_SINGLE_VALUE(isp_test_settings, isp_exp_line);
SET_ISP_SINGLE_VALUE(isp_test_settings, isp_color_temp);
SET_ISP_SINGLE_VALUE(isp_test_settings, ae_forced);
SET_ISP_SINGLE_VALUE(isp_test_settings, lum_forced);
SET_ISP_SINGLE_VALUE(isp_test_settings, isp_version);

SET_ISP_SINGLE_VALUE(isp_test_settings, manual_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, afs_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, ae_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, af_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, awb_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, hist_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, wdr_split_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, wdr_stitch_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, fpn_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, dpc_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, ctc_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, gca_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, nrp_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, denoise_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, tdf_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, blc_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, wb_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, dig_gain_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, lsc_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, msc_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, pltm_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, cfa_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, lca_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, sharp_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, ccm_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, defog_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, drc_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, gtm_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, gamma_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, cem_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, encpp_top_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, encpp_sharp_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, encpp_ldci_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, enc_3dnr_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, enc_2dnr_en);

SET_ISP_SINGLE_VALUE(isp_3a_settings, define_ae_table);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_max_lv);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_table_preview_length);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_table_capture_length);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_table_video_length);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_hist_mod_en);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_hist0_sel);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_hist1_sel);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_stat_sel);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_stat2_sel);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_ev_step);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_ConvDataIndex);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_flicker_comp_en);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_flicker_comp_max);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_w_num);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_h_num);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_gain_favor);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_fastboot_mod);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_reserve_0);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_reserve_1);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_reserve_2);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_reserve_3);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_reserve_4);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_delay_frame);
SET_ISP_SINGLE_VALUE(isp_3a_settings, exp_delay_frame);
SET_ISP_SINGLE_VALUE(isp_3a_settings, gain_delay_frame);
SET_ISP_SINGLE_VALUE(isp_3a_settings, exp_comp_step);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_touch_dist_ind);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_iso2gain_ratio);

SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_interval);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_speed);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_stat_sel);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_w_num);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_h_num);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_stat_low_lim);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_stat_high_lim);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_color_temper_low);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_color_temper_high);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_base_temper);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_green_zone_dist);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_blue_sky_dist);

SET_ISP_SINGLE_VALUE(isp_3a_settings, local_wb_coef);
SET_ISP_SINGLE_VALUE(isp_3a_settings, complex_light_sat_coef);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_stat_mode);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_reserve_0);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_reserve_1);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_reserve_2);

SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_light_num);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_ext_light_num);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_skin_color_num);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_special_color_num);

SET_ISP_SINGLE_VALUE(isp_3a_settings, af_use_otp);
SET_ISP_SINGLE_VALUE(isp_3a_settings, vcm_min_code);
SET_ISP_SINGLE_VALUE(isp_3a_settings, vcm_max_code);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_interval_time);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_speed_ind);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_auto_fine_en);

SET_ISP_SINGLE_VALUE(isp_3a_settings, af_single_fine_en);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_fine_step);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_reserve_0);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_reserve_1);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_reserve_2);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_reserve_3);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_move_cnt);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_still_cnt);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_move_monitor_cnt);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_still_monitor_cnt);

SET_ISP_SINGLE_VALUE(isp_3a_settings, af_stable_min);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_stable_max);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_low_light_lv);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_near_tolerance);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_far_tolerance);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_tolerance_off);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_peak_th);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_dir_th);

SET_ISP_SINGLE_VALUE(isp_3a_settings, af_change_ratio);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_move_minus);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_still_minus);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_scene_motion_th);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_tolerance_tbl_len);

SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_en);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_width);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_height);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_pd_w_num);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_pd_h_num);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_dcc_map_w_num);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_dcc_map_h_num);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_gain_map_w_num);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_gain_map_h_num);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_win_conf_th);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_weight_conf_th);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_defocus_begin_th);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_max_step);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_defocus_step0_th);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_defocus_step0_coef);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_defocus_step1_th);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_defocus_step1_coef);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_defocus_step2_th);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_defocus_step2_coef);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_defocus_continue_th);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_delay);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_pd_err_th1);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_pd_err_th2);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_algo_type);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_algo_touch_en);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_algo_conf_th);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_algo_blk_calc_times);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_algo_conf_coef);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_pdaf_algo_overexp_th);

SET_ISP_SINGLE_VALUE(isp_tunning_settings, flash_gain);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, flash_delay_frame);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, hor_visual_angle);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, ver_visual_angle);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, focus_length);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, gamma_num);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, rolloff_ratio);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, flicker_type);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, flicker_ratio);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, flicker_local_comp);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, afs_sel);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, afs_low_th);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, afs_high_th);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, gtm_hist_sel);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, gtm_type);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, gtm_bright_dn_limit);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, gtm_interval);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, gtm_reserve0);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, gtm_reserve1);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, gtm_reserve2);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, gtm_reserve3);

SET_ISP_SINGLE_VALUE(isp_tunning_settings, grad_th);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, dir_v_th);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, dir_h_th);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, res_smth_high);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, res_smth_low);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, res_high_th);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, res_low_th);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, res_dir_a);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, res_dir_d);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, res_dir_v);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, res_dir_h);

SET_ISP_SINGLE_VALUE(isp_tunning_settings, fpn_pfpn_en);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, fpn_cfpn_en);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, fpn_pfpn_cluster_size);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, fpn_cfpn_lw_th);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, fpn_cfpn_hi_th);

SET_ISP_SINGLE_VALUE(isp_tunning_settings, ctc_low_th_min);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, ctc_low_th);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, ctc_high_th);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, ctc_high_th_max);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, ctc_dir_wt);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, ctc_dir_th);

SET_ISP_SINGLE_VALUE(isp_tunning_settings, ff_mod);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, lsc_mode);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, lsc_center_x);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, lsc_center_y);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, mff_mod);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, msc_mode);

SET_ISP_ARRAY_VALUE(isp_3a_settings, ae_table_preview);
SET_ISP_ARRAY_VALUE(isp_3a_settings, ae_table_capture);
SET_ISP_ARRAY_VALUE(isp_3a_settings, ae_table_video);
SET_ISP_ARRAY_VALUE(isp_3a_settings, ae_win_weight);
SET_ISP_ARRAY_VALUE(isp_3a_settings, ae_fno_step);
SET_ISP_ARRAY_VALUE(isp_3a_settings, wdr_split_cfg);
SET_ISP_ARRAY_VALUE(isp_3a_settings, wdr_comm_cfg);
SET_ISP_ARRAY_VALUE(isp_3a_settings, awb_light_info);
SET_ISP_ARRAY_VALUE(isp_3a_settings, awb_ext_light_info);
SET_ISP_ARRAY_VALUE(isp_3a_settings, awb_skin_color_info);
SET_ISP_ARRAY_VALUE(isp_3a_settings, awb_special_color_info);
SET_ISP_ARRAY_VALUE(isp_3a_settings, awb_preset_gain);
SET_ISP_ARRAY_VALUE(isp_3a_settings, af_std_code_tbl);
SET_ISP_ARRAY_VALUE(isp_3a_settings, af_tolerance_value_tbl);
SET_ISP_ARRAY_VALUE(isp_3a_settings, af_pdaf_weight_win);
SET_ISP_ARRAY_VALUE(isp_3a_settings, af_pdaf_algo_calc_tbl);

SET_ISP_ARRAY_VALUE(isp_tunning_settings, fpn_pfpn_phase_lut);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, fpn_pfpn_period_lut);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, fpn_pfpn_offset_lut);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, dpc_comm_cfg);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, bayer_gain);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, lsc_trig_cfg);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, msc_trig_cfg);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, msc_blw_lut);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, msc_blh_lut);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, gamma_trig_cfg);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, ccm_trig_cfg);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, lca_comm_cfg);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, gca_cfg);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, pltm_cfg);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, sharp_comm_cfg);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, denoise_comm_cfg);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, tdf_comm_cfg);
#ifdef USE_ENCPP
SET_ISP_ARRAY_VALUE(isp_tunning_settings, encpp_top_comm_cfg);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, encpp_sharp_comm_cfg);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, encpp_ldci_comm_cfg);
#endif
SET_ISP_ARRAY_VALUE(isp_tunning_settings, nrp_comm_cfg);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, sensor_temp);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_d2d_nr_cbth);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_d2d_nr_crth);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_d2d_lp0_nr_ybk);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_d2d_lp1_nr_ybk);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_d2d_lp2_nr_ybk);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_tdnf_df_shape);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_tdnf_ratio_amp);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_tdnf_k_dlt_bk);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_tdnf_ct_rt_bk);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_tdnf_dtc_hf_bk);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_tdnf_dtc_mf_bk);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_tdnf_dtc_hf_ba);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_tdnf_dtc_mf_ba);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_tdnf_lay0_d2d0_rt_br);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_tdnf_lay1_d2d0_rt_br);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_tdnf_lay0_nrd_rt_br);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_tdnf_lay1_nrd_rt_br);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, lca_pf_satu_lut);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, lca_gf_satu_lut);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_sharp_hs_value);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_sharp_ms_value);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_sharp_ls_value);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_sharp_hs_lum);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_sharp_ms_lum);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_sharp_ls_lum);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_sharp_hsv);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, sharp_ratio_by_mot);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, sharp_ratio_by_tex);
#ifdef USE_ENCPP
SET_ISP_ARRAY_VALUE(isp_tunning_settings, encpp_gbl_satu_adj_lut);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, encpp_sharp_hs_value);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, encpp_sharp_ms_value);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, encpp_sharp_hs_lum);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, encpp_sharp_ms_lum);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, encpp_sharp_hsv);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, encpp_sharp_ls_map_lut);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, encpp_sharp_texture_lut);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, encpp_sharp_ratio_by_mot);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, encpp_sharp_ratio_by_tex);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, encpp_ldci_txt_ratio_lut);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, encpp_ldci_mot_ratio_lut);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, encpp_ldci_lum_diff_lut);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, encpp_ldci_map_pos_lut);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, encpp_ldci_map_neg_lut);
#endif
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_wdr_de_purpl_hsv_tbl);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_pltm_stat_gd_cv);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_pltm_df_cv);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, fpn_cfpn_coeff_lut);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, fpn_pfpn_scale_coeff_lut);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_pltm_lum_map_cv);

SET_ISP_STRUCT_CHAR(isp_iso_settings, triger);
SET_ISP_ARRAY_INT(isp_iso_settings, isp_lum_mapping_point);
SET_ISP_ARRAY_INT(isp_iso_settings, isp_gain_mapping_point);

SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, sharp_cfg, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, sharp_cfg, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, sharp_cfg, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, sharp_cfg, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, sharp_cfg, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, sharp_cfg, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, sharp_cfg, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, sharp_cfg, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, sharp_cfg, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, sharp_cfg, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, sharp_cfg, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, sharp_cfg, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, sharp_cfg, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, sharp_cfg, 13);
#ifdef USE_ENCPP
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_sharp_cfg, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_sharp_cfg, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_sharp_cfg, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_sharp_cfg, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_sharp_cfg, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_sharp_cfg, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_sharp_cfg, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_sharp_cfg, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_sharp_cfg, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_sharp_cfg, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_sharp_cfg, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_sharp_cfg, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_sharp_cfg, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_sharp_cfg, 13);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_ldci_cfg, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_ldci_cfg, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_ldci_cfg, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_ldci_cfg, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_ldci_cfg, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_ldci_cfg, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_ldci_cfg, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_ldci_cfg, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_ldci_cfg, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_ldci_cfg, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_ldci_cfg, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_ldci_cfg, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_ldci_cfg, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_ldci_cfg, 13);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_top_cfg, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_top_cfg, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_top_cfg, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_top_cfg, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_top_cfg, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_top_cfg, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_top_cfg, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_top_cfg, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_top_cfg, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_top_cfg, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_top_cfg, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_top_cfg, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_top_cfg, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encpp_top_cfg, 13);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encoder_denoise_cfg, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encoder_denoise_cfg, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encoder_denoise_cfg, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encoder_denoise_cfg, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encoder_denoise_cfg, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encoder_denoise_cfg, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encoder_denoise_cfg, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encoder_denoise_cfg, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encoder_denoise_cfg, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encoder_denoise_cfg, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encoder_denoise_cfg, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encoder_denoise_cfg, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encoder_denoise_cfg, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, encoder_denoise_cfg, 13);
#endif
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, denoise_cfg, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, denoise_cfg, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, denoise_cfg, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, denoise_cfg, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, denoise_cfg, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, denoise_cfg, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, denoise_cfg, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, denoise_cfg, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, denoise_cfg, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, denoise_cfg, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, denoise_cfg, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, denoise_cfg, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, denoise_cfg, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, denoise_cfg, 13);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, black_level, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, black_level, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, black_level, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, black_level, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, black_level, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, black_level, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, black_level, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, black_level, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, black_level, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, black_level, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, black_level, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, black_level, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, black_level, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, black_level, 13);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, dpc_cfg, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, dpc_cfg, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, dpc_cfg, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, dpc_cfg, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, dpc_cfg, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, dpc_cfg, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, dpc_cfg, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, dpc_cfg, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, dpc_cfg, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, dpc_cfg, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, dpc_cfg, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, dpc_cfg, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, dpc_cfg, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, dpc_cfg, 13);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, pltm_dynamic_cfg, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, pltm_dynamic_cfg, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, pltm_dynamic_cfg, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, pltm_dynamic_cfg, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, pltm_dynamic_cfg, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, pltm_dynamic_cfg, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, pltm_dynamic_cfg, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, pltm_dynamic_cfg, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, pltm_dynamic_cfg, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, pltm_dynamic_cfg, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, pltm_dynamic_cfg, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, pltm_dynamic_cfg, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, pltm_dynamic_cfg, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, pltm_dynamic_cfg, 13);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, defog_value, 0);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, defog_value, 1);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, defog_value, 2);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, defog_value, 3);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, defog_value, 4);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, defog_value, 5);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, defog_value, 6);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, defog_value, 7);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, defog_value, 8);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, defog_value, 9);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, defog_value, 10);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, defog_value, 11);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, defog_value, 12);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, defog_value, 13);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, brightness, 0);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, brightness, 1);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, brightness, 2);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, brightness, 3);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, brightness, 4);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, brightness, 5);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, brightness, 6);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, brightness, 7);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, brightness, 8);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, brightness, 9);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, brightness, 10);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, brightness, 11);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, brightness, 12);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, brightness, 13);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, contrast, 0);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, contrast, 1);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, contrast, 2);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, contrast, 3);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, contrast, 4);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, contrast, 5);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, contrast, 6);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, contrast, 7);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, contrast, 8);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, contrast, 9);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, contrast, 10);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, contrast, 11);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, contrast, 12);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, contrast, 13);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cem_cfg, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cem_cfg, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cem_cfg, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cem_cfg, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cem_cfg, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cem_cfg, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cem_cfg, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cem_cfg, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cem_cfg, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cem_cfg, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cem_cfg, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cem_cfg, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cem_cfg, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cem_cfg, 13);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, tdf_cfg, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, tdf_cfg, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, tdf_cfg, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, tdf_cfg, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, tdf_cfg, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, tdf_cfg, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, tdf_cfg, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, tdf_cfg, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, tdf_cfg, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, tdf_cfg, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, tdf_cfg, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, tdf_cfg, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, tdf_cfg, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, tdf_cfg, 13);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, ae_cfg, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, ae_cfg, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, ae_cfg, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, ae_cfg, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, ae_cfg, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, ae_cfg, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, ae_cfg, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, ae_cfg, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, ae_cfg, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, ae_cfg, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, ae_cfg, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, ae_cfg, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, ae_cfg, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, ae_cfg, 13);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, gtm_cfg, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, gtm_cfg, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, gtm_cfg, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, gtm_cfg, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, gtm_cfg, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, gtm_cfg, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, gtm_cfg, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, gtm_cfg, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, gtm_cfg, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, gtm_cfg, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, gtm_cfg, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, gtm_cfg, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, gtm_cfg, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, gtm_cfg, 13);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, lca_cfg, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, lca_cfg, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, lca_cfg, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, lca_cfg, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, lca_cfg, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, lca_cfg, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, lca_cfg, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, lca_cfg, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, lca_cfg, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, lca_cfg, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, lca_cfg, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, lca_cfg, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, lca_cfg, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, lca_cfg, 13);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, wdr_cfg, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, wdr_cfg, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, wdr_cfg, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, wdr_cfg, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, wdr_cfg, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, wdr_cfg, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, wdr_cfg, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, wdr_cfg, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, wdr_cfg, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, wdr_cfg, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, wdr_cfg, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, wdr_cfg, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, wdr_cfg, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, wdr_cfg, 13);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cfa_cfg, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cfa_cfg, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cfa_cfg, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cfa_cfg, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cfa_cfg, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cfa_cfg, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cfa_cfg, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cfa_cfg, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cfa_cfg, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cfa_cfg, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cfa_cfg, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cfa_cfg, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cfa_cfg, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, cfa_cfg, 13);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, shading_comp, 0);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, shading_comp, 1);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, shading_comp, 2);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, shading_comp, 3);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, shading_comp, 4);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, shading_comp, 5);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, shading_comp, 6);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, shading_comp, 7);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, shading_comp, 8);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, shading_comp, 9);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, shading_comp, 10);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, shading_comp, 11);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, shading_comp, 12);
SET_ISP_DYNAMIC_SINGLE_VALUE(isp_iso_settings, shading_comp, 13);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, awb_cfg, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, awb_cfg, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, awb_cfg, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, awb_cfg, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, awb_cfg, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, awb_cfg, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, awb_cfg, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, awb_cfg, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, awb_cfg, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, awb_cfg, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, awb_cfg, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, awb_cfg, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, awb_cfg, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, awb_cfg, 13);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, nrp_cfg, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, nrp_cfg, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, nrp_cfg, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, nrp_cfg, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, nrp_cfg, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, nrp_cfg, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, nrp_cfg, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, nrp_cfg, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, nrp_cfg, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, nrp_cfg, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, nrp_cfg, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, nrp_cfg, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, nrp_cfg, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, nrp_cfg, 13);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, af_cfg, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, af_cfg, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, af_cfg, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, af_cfg, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, af_cfg, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, af_cfg, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, af_cfg, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, af_cfg, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, af_cfg, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, af_cfg, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, af_cfg, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, af_cfg, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, af_cfg, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, af_cfg, 13);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp0_nr_yth, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp0_nr_yth, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp0_nr_yth, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp0_nr_yth, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp0_nr_yth, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp0_nr_yth, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp0_nr_yth, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp0_nr_yth, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp0_nr_yth, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp0_nr_yth, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp0_nr_yth, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp0_nr_yth, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp0_nr_yth, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp0_nr_yth, 13);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp1_nr_yth, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp1_nr_yth, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp1_nr_yth, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp1_nr_yth, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp1_nr_yth, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp1_nr_yth, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp1_nr_yth, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp1_nr_yth, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp1_nr_yth, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp1_nr_yth, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp1_nr_yth, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp1_nr_yth, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp1_nr_yth, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp1_nr_yth, 13);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp2_nr_yth, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp2_nr_yth, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp2_nr_yth, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp2_nr_yth, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp2_nr_yth, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp2_nr_yth, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp2_nr_yth, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp2_nr_yth, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp2_nr_yth, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp2_nr_yth, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp2_nr_yth, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp2_nr_yth, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp2_nr_yth, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d2d_lp2_nr_yth, 13);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d3d_flt0_thr_vc, 0);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d3d_flt0_thr_vc, 1);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d3d_flt0_thr_vc, 2);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d3d_flt0_thr_vc, 3);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d3d_flt0_thr_vc, 4);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d3d_flt0_thr_vc, 5);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d3d_flt0_thr_vc, 6);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d3d_flt0_thr_vc, 7);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d3d_flt0_thr_vc, 8);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d3d_flt0_thr_vc, 9);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d3d_flt0_thr_vc, 10);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d3d_flt0_thr_vc, 11);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d3d_flt0_thr_vc, 12);
SET_ISP_DYNAMIC_ARRAY_SHORT(isp_iso_settings, d3d_flt0_thr_vc, 13);

SET_ISP_CM_VALUE(isp_color_matrix, 0);
SET_ISP_CM_VALUE(isp_color_matrix, 1);
SET_ISP_CM_VALUE(isp_color_matrix, 2);

struct IspParamAttribute {
	char *main;
	char *sub;
	int len;
	void (*set_param) (struct isp_param_config *, void *, int len);
};

struct FileAttribute {
	char *file_name;
	int param_len;
	struct IspParamAttribute *pIspParam;
};

static struct IspParamAttribute IspTestParam[] = {

	ISP_FILE_SINGLE_ATTR(isp_test_cfg, isp_test_mode),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, isp_test_exptime),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, exp_line_start),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, exp_line_step),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, exp_line_end),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, exp_change_interval),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, isp_test_gain),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, gain_start),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, gain_step),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, gain_end),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, gain_change_interval),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, isp_test_focus),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, focus_start),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, focus_step),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, focus_end),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, focus_change_interval),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, isp_log_param),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, isp_gain),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, isp_exp_line),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, isp_color_temp),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, ae_forced),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, lum_forced),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, isp_version),

	ISP_FILE_SINGLE_ATTR(isp_en_cfg, manual_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, afs_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, ae_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, af_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, awb_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, hist_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, wdr_split_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, wdr_stitch_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, fpn_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, dpc_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, ctc_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, gca_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, nrp_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, denoise_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, tdf_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, blc_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, wb_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, dig_gain_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, lsc_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, msc_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, pltm_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, cfa_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, lca_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, sharp_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, ccm_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, defog_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, drc_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, gtm_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, gamma_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, cem_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, encpp_top_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, encpp_sharp_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, encpp_ldci_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, enc_3dnr_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, enc_2dnr_en),
};

static struct IspParamAttribute Isp3aParam[] = {
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, define_ae_table),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_max_lv),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_table_preview_length),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_table_capture_length),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_table_video_length),

	ISP_FILE_ARRAY_ATTR(isp_ae_cfg, ae_table_preview, 42),
	ISP_FILE_ARRAY_ATTR(isp_ae_cfg, ae_table_capture, 42),
	ISP_FILE_ARRAY_ATTR(isp_ae_cfg, ae_table_video, 42),
	ISP_FILE_ARRAY_ATTR(isp_ae_cfg, ae_win_weight, 64),
	ISP_FILE_ARRAY_ATTR(isp_ae_cfg, ae_fno_step, 16),

	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_hist_mod_en),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_hist0_sel),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_hist1_sel),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_stat_sel),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_stat2_sel),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_ev_step),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_ConvDataIndex),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_flicker_comp_en),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_flicker_comp_max),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_w_num),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_h_num),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_gain_favor),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_fastboot_mod),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_reserve_0),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_reserve_1),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_reserve_2),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_reserve_3),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_reserve_4),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_delay_frame),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, exp_delay_frame),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, gain_delay_frame),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, exp_comp_step),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_touch_dist_ind),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_iso2gain_ratio),
	ISP_FILE_ARRAY_ATTR(isp_ae_cfg, wdr_split_cfg, ISP_WDR_SPLIT_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_ae_cfg, wdr_comm_cfg, ISP_WDR_COMM_CFG_MAX),

	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_interval),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_speed),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_stat_sel),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_w_num),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_h_num),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_stat_low_lim),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_stat_high_lim),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_color_temper_low),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_color_temper_high),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_base_temper),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_green_zone_dist),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_blue_sky_dist),

	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_light_num),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_ext_light_num),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_skin_color_num),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_special_color_num),

	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, local_wb_coef),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, complex_light_sat_coef),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_stat_mode),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_reserve_0),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_reserve_1),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_reserve_2),

	ISP_FILE_ARRAY_ATTR(isp_awb_cfg, awb_light_info, 160),
	ISP_FILE_ARRAY_ATTR(isp_awb_cfg, awb_ext_light_info, 80),
	ISP_FILE_ARRAY_ATTR(isp_awb_cfg, awb_skin_color_info, 40),
	ISP_FILE_ARRAY_ATTR(isp_awb_cfg, awb_special_color_info, 80),
	ISP_FILE_ARRAY_ATTR(isp_awb_cfg, awb_preset_gain, 22),

	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_use_otp),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, vcm_min_code),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, vcm_max_code),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_interval_time),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_speed_ind),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_auto_fine_en),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_single_fine_en),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_fine_step),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_reserve_0),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_reserve_1),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_reserve_2),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_reserve_3),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_move_cnt),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_still_cnt),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_move_monitor_cnt),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_still_monitor_cnt),

	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_stable_min),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_stable_max),

	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_low_light_lv),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_near_tolerance),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_far_tolerance),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_tolerance_off),

	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_peak_th),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_dir_th),

	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_change_ratio),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_move_minus),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_still_minus),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_scene_motion_th),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_tolerance_tbl_len),

	ISP_FILE_ARRAY_ATTR(isp_af_cfg, af_std_code_tbl, 20),
	ISP_FILE_ARRAY_ATTR(isp_af_cfg, af_tolerance_value_tbl, 20),

	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_en),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_width),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_height),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_pd_w_num),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_pd_h_num),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_dcc_map_w_num),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_dcc_map_h_num),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_gain_map_w_num),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_gain_map_h_num),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_win_conf_th),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_weight_conf_th),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_defocus_begin_th),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_max_step),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_defocus_step0_th),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_defocus_step0_coef),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_defocus_step1_th),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_defocus_step1_coef),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_defocus_step2_th),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_defocus_step2_coef),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_defocus_continue_th),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_delay),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_pd_err_th1),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_pd_err_th2),
	ISP_FILE_ARRAY_ATTR(isp_af_cfg, af_pdaf_weight_win, 64),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_algo_type),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_algo_touch_en),
	ISP_FILE_ARRAY_ATTR(isp_af_cfg, af_pdaf_algo_calc_tbl, 48),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_algo_conf_th),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_algo_blk_calc_times),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_algo_conf_coef),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_pdaf_algo_overexp_th),
};

static struct IspParamAttribute IspDynamicParam[] = {
#ifdef USE_ENCPP
	ISP_FILE_ARRAY_ATTR(isp_dynamic_gbl, triger, 23),
#else
	ISP_FILE_ARRAY_ATTR(isp_dynamic_gbl, triger, 19),
#endif
	ISP_FILE_ARRAY_ATTR(isp_dynamic_gbl, isp_lum_mapping_point, 14),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_gbl, isp_gain_mapping_point, 14),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  sharp_cfg_0, ISP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  sharp_cfg_1, ISP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  sharp_cfg_2, ISP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  sharp_cfg_3, ISP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  sharp_cfg_4, ISP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  sharp_cfg_5, ISP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  sharp_cfg_6, ISP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  sharp_cfg_7, ISP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  sharp_cfg_8, ISP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  sharp_cfg_9, ISP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, sharp_cfg_10, ISP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, sharp_cfg_11, ISP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, sharp_cfg_12, ISP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, sharp_cfg_13, ISP_SHARP_MAX),
#ifdef USE_ENCPP
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  encpp_sharp_cfg_0, ENCPP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  encpp_sharp_cfg_1, ENCPP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  encpp_sharp_cfg_2, ENCPP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  encpp_sharp_cfg_3, ENCPP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  encpp_sharp_cfg_4, ENCPP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  encpp_sharp_cfg_5, ENCPP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  encpp_sharp_cfg_6, ENCPP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  encpp_sharp_cfg_7, ENCPP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  encpp_sharp_cfg_8, ENCPP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  encpp_sharp_cfg_9, ENCPP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, encpp_sharp_cfg_10, ENCPP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, encpp_sharp_cfg_11, ENCPP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, encpp_sharp_cfg_12, ENCPP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, encpp_sharp_cfg_13, ENCPP_SHARP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  encpp_ldci_cfg_0, ENCPP_LDCI_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  encpp_ldci_cfg_1, ENCPP_LDCI_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  encpp_ldci_cfg_2, ENCPP_LDCI_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  encpp_ldci_cfg_3, ENCPP_LDCI_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  encpp_ldci_cfg_4, ENCPP_LDCI_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  encpp_ldci_cfg_5, ENCPP_LDCI_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  encpp_ldci_cfg_6, ENCPP_LDCI_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  encpp_ldci_cfg_7, ENCPP_LDCI_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  encpp_ldci_cfg_8, ENCPP_LDCI_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  encpp_ldci_cfg_9, ENCPP_LDCI_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, encpp_ldci_cfg_10, ENCPP_LDCI_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, encpp_ldci_cfg_11, ENCPP_LDCI_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, encpp_ldci_cfg_12, ENCPP_LDCI_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, encpp_ldci_cfg_13, ENCPP_LDCI_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  encpp_top_cfg_0, ENCPP_TOP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  encpp_top_cfg_1, ENCPP_TOP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  encpp_top_cfg_2, ENCPP_TOP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  encpp_top_cfg_3, ENCPP_TOP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  encpp_top_cfg_4, ENCPP_TOP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  encpp_top_cfg_5, ENCPP_TOP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  encpp_top_cfg_6, ENCPP_TOP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  encpp_top_cfg_7, ENCPP_TOP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  encpp_top_cfg_8, ENCPP_TOP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  encpp_top_cfg_9, ENCPP_TOP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, encpp_top_cfg_10, ENCPP_TOP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, encpp_top_cfg_11, ENCPP_TOP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, encpp_top_cfg_12, ENCPP_TOP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, encpp_top_cfg_13, ENCPP_TOP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  encoder_denoise_cfg_0, ENCODER_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  encoder_denoise_cfg_1, ENCODER_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  encoder_denoise_cfg_2, ENCODER_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  encoder_denoise_cfg_3, ENCODER_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  encoder_denoise_cfg_4, ENCODER_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  encoder_denoise_cfg_5, ENCODER_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  encoder_denoise_cfg_6, ENCODER_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  encoder_denoise_cfg_7, ENCODER_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  encoder_denoise_cfg_8, ENCODER_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  encoder_denoise_cfg_9, ENCODER_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, encoder_denoise_cfg_10, ENCODER_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, encoder_denoise_cfg_11, ENCODER_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, encoder_denoise_cfg_12, ENCODER_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, encoder_denoise_cfg_13, ENCODER_DENOISE_MAX),
#endif
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  denoise_cfg_0, ISP_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  denoise_cfg_1, ISP_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  denoise_cfg_2, ISP_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  denoise_cfg_3, ISP_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  denoise_cfg_4, ISP_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  denoise_cfg_5, ISP_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  denoise_cfg_6, ISP_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  denoise_cfg_7, ISP_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  denoise_cfg_8, ISP_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  denoise_cfg_9, ISP_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, denoise_cfg_10, ISP_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, denoise_cfg_11, ISP_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, denoise_cfg_12, ISP_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, denoise_cfg_13, ISP_DENOISE_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  black_level_0, ISP_BLC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  black_level_1, ISP_BLC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  black_level_2, ISP_BLC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  black_level_3, ISP_BLC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  black_level_4, ISP_BLC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  black_level_5, ISP_BLC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  black_level_6, ISP_BLC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  black_level_7, ISP_BLC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  black_level_8, ISP_BLC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  black_level_9, ISP_BLC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, black_level_10, ISP_BLC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, black_level_11, ISP_BLC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, black_level_12, ISP_BLC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, black_level_13, ISP_BLC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  dpc_cfg_0, ISP_DPC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  dpc_cfg_1, ISP_DPC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  dpc_cfg_2, ISP_DPC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  dpc_cfg_3, ISP_DPC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  dpc_cfg_4, ISP_DPC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  dpc_cfg_5, ISP_DPC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  dpc_cfg_6, ISP_DPC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  dpc_cfg_7, ISP_DPC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  dpc_cfg_8, ISP_DPC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  dpc_cfg_9, ISP_DPC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, dpc_cfg_10, ISP_DPC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, dpc_cfg_11, ISP_DPC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, dpc_cfg_12, ISP_DPC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, dpc_cfg_13, ISP_DPC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  pltm_dynamic_cfg_0, ISP_PLTM_DYNAMIC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  pltm_dynamic_cfg_1, ISP_PLTM_DYNAMIC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  pltm_dynamic_cfg_2, ISP_PLTM_DYNAMIC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  pltm_dynamic_cfg_3, ISP_PLTM_DYNAMIC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  pltm_dynamic_cfg_4, ISP_PLTM_DYNAMIC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  pltm_dynamic_cfg_5, ISP_PLTM_DYNAMIC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  pltm_dynamic_cfg_6, ISP_PLTM_DYNAMIC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  pltm_dynamic_cfg_7, ISP_PLTM_DYNAMIC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  pltm_dynamic_cfg_8, ISP_PLTM_DYNAMIC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  pltm_dynamic_cfg_9, ISP_PLTM_DYNAMIC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, pltm_dynamic_cfg_10, ISP_PLTM_DYNAMIC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, pltm_dynamic_cfg_11, ISP_PLTM_DYNAMIC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, pltm_dynamic_cfg_12, ISP_PLTM_DYNAMIC_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, pltm_dynamic_cfg_13, ISP_PLTM_DYNAMIC_MAX),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_0,  defog_value_0),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_1,  defog_value_1),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_2,  defog_value_2),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_3,  defog_value_3),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_4,  defog_value_4),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_5,  defog_value_5),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_6,  defog_value_6),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_7,  defog_value_7),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_8,  defog_value_8),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_9,  defog_value_9),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_10, defog_value_10),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_11, defog_value_11),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_12, defog_value_12),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_13, defog_value_13),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_0,  brightness_0),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_1,  brightness_1),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_2,  brightness_2),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_3,  brightness_3),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_4,  brightness_4),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_5,  brightness_5),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_6,  brightness_6),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_7,  brightness_7),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_8,  brightness_8),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_9,  brightness_9),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_10, brightness_10),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_11, brightness_11),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_12, brightness_12),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_13, brightness_13),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_0,  contrast_0),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_1,  contrast_1),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_2,  contrast_2),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_3,  contrast_3),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_4,  contrast_4),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_5,  contrast_5),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_6,  contrast_6),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_7,  contrast_7),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_8,  contrast_8),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_9,  contrast_9),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_10, contrast_10),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_11, contrast_11),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_12, contrast_12),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_13, contrast_13),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  cem_cfg_0, ISP_CEM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  cem_cfg_1, ISP_CEM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  cem_cfg_2, ISP_CEM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  cem_cfg_3, ISP_CEM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  cem_cfg_4, ISP_CEM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  cem_cfg_5, ISP_CEM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  cem_cfg_6, ISP_CEM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  cem_cfg_7, ISP_CEM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  cem_cfg_8, ISP_CEM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  cem_cfg_9, ISP_CEM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, cem_cfg_10, ISP_CEM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, cem_cfg_11, ISP_CEM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, cem_cfg_12, ISP_CEM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, cem_cfg_13, ISP_CEM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  tdf_cfg_0, ISP_TDF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  tdf_cfg_1, ISP_TDF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  tdf_cfg_2, ISP_TDF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  tdf_cfg_3, ISP_TDF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  tdf_cfg_4, ISP_TDF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  tdf_cfg_5, ISP_TDF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  tdf_cfg_6, ISP_TDF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  tdf_cfg_7, ISP_TDF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  tdf_cfg_8, ISP_TDF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  tdf_cfg_9, ISP_TDF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, tdf_cfg_10, ISP_TDF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, tdf_cfg_11, ISP_TDF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, tdf_cfg_12, ISP_TDF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, tdf_cfg_13, ISP_TDF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  ae_cfg_0, ISP_EXP_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  ae_cfg_1, ISP_EXP_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  ae_cfg_2, ISP_EXP_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  ae_cfg_3, ISP_EXP_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  ae_cfg_4, ISP_EXP_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  ae_cfg_5, ISP_EXP_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  ae_cfg_6, ISP_EXP_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  ae_cfg_7, ISP_EXP_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  ae_cfg_8, ISP_EXP_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  ae_cfg_9, ISP_EXP_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, ae_cfg_10, ISP_EXP_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, ae_cfg_11, ISP_EXP_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, ae_cfg_12, ISP_EXP_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, ae_cfg_13, ISP_EXP_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  gtm_cfg_0, ISP_GTM_HEQ_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  gtm_cfg_1, ISP_GTM_HEQ_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  gtm_cfg_2, ISP_GTM_HEQ_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  gtm_cfg_3, ISP_GTM_HEQ_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  gtm_cfg_4, ISP_GTM_HEQ_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  gtm_cfg_5, ISP_GTM_HEQ_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  gtm_cfg_6, ISP_GTM_HEQ_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  gtm_cfg_7, ISP_GTM_HEQ_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  gtm_cfg_8, ISP_GTM_HEQ_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  gtm_cfg_9, ISP_GTM_HEQ_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, gtm_cfg_10, ISP_GTM_HEQ_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, gtm_cfg_11, ISP_GTM_HEQ_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, gtm_cfg_12, ISP_GTM_HEQ_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, gtm_cfg_13, ISP_GTM_HEQ_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  lca_cfg_0, ISP_LCA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  lca_cfg_1, ISP_LCA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  lca_cfg_2, ISP_LCA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  lca_cfg_3, ISP_LCA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  lca_cfg_4, ISP_LCA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  lca_cfg_5, ISP_LCA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  lca_cfg_6, ISP_LCA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  lca_cfg_7, ISP_LCA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  lca_cfg_8, ISP_LCA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  lca_cfg_9, ISP_LCA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, lca_cfg_10, ISP_LCA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, lca_cfg_11, ISP_LCA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, lca_cfg_12, ISP_LCA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, lca_cfg_13, ISP_LCA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  wdr_cfg_0, ISP_WDR_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  wdr_cfg_1, ISP_WDR_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  wdr_cfg_2, ISP_WDR_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  wdr_cfg_3, ISP_WDR_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  wdr_cfg_4, ISP_WDR_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  wdr_cfg_5, ISP_WDR_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  wdr_cfg_6, ISP_WDR_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  wdr_cfg_7, ISP_WDR_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  wdr_cfg_8, ISP_WDR_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  wdr_cfg_9, ISP_WDR_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, wdr_cfg_10, ISP_WDR_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, wdr_cfg_11, ISP_WDR_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, wdr_cfg_12, ISP_WDR_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, wdr_cfg_13, ISP_WDR_CFG_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  cfa_cfg_0, ISP_CFA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  cfa_cfg_1, ISP_CFA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  cfa_cfg_2, ISP_CFA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  cfa_cfg_3, ISP_CFA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  cfa_cfg_4, ISP_CFA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  cfa_cfg_5, ISP_CFA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  cfa_cfg_6, ISP_CFA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  cfa_cfg_7, ISP_CFA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  cfa_cfg_8, ISP_CFA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  cfa_cfg_9, ISP_CFA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, cfa_cfg_10, ISP_CFA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, cfa_cfg_11, ISP_CFA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, cfa_cfg_12, ISP_CFA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, cfa_cfg_13, ISP_CFA_MAX),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_0,  shading_comp_0),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_1,  shading_comp_1),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_2,  shading_comp_2),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_3,  shading_comp_3),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_4,  shading_comp_4),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_5,  shading_comp_5),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_6,  shading_comp_6),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_7,  shading_comp_7),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_8,  shading_comp_8),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_9,  shading_comp_9),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_10, shading_comp_10),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_11, shading_comp_11),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_12, shading_comp_12),
	ISP_FILE_SINGLE_ATTR(isp_dynamic_cfg_13, shading_comp_13),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  awb_cfg_0, ISP_AWB_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  awb_cfg_1, ISP_AWB_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  awb_cfg_2, ISP_AWB_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  awb_cfg_3, ISP_AWB_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  awb_cfg_4, ISP_AWB_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  awb_cfg_5, ISP_AWB_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  awb_cfg_6, ISP_AWB_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  awb_cfg_7, ISP_AWB_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  awb_cfg_8, ISP_AWB_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  awb_cfg_9, ISP_AWB_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, awb_cfg_10, ISP_AWB_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, awb_cfg_11, ISP_AWB_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, awb_cfg_12, ISP_AWB_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, awb_cfg_13, ISP_AWB_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  nrp_cfg_0, ISP_NRP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  nrp_cfg_1, ISP_NRP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  nrp_cfg_2, ISP_NRP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  nrp_cfg_3, ISP_NRP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  nrp_cfg_4, ISP_NRP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  nrp_cfg_5, ISP_NRP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  nrp_cfg_6, ISP_NRP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  nrp_cfg_7, ISP_NRP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  nrp_cfg_8, ISP_NRP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  nrp_cfg_9, ISP_NRP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, nrp_cfg_10, ISP_NRP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, nrp_cfg_11, ISP_NRP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, nrp_cfg_12, ISP_NRP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, nrp_cfg_13, ISP_NRP_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  af_cfg_0, ISP_AF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  af_cfg_1, ISP_AF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  af_cfg_2, ISP_AF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  af_cfg_3, ISP_AF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  af_cfg_4, ISP_AF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  af_cfg_5, ISP_AF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  af_cfg_6, ISP_AF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  af_cfg_7, ISP_AF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  af_cfg_8, ISP_AF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  af_cfg_9, ISP_AF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, af_cfg_10, ISP_AF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, af_cfg_11, ISP_AF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, af_cfg_12, ISP_AF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, af_cfg_13, ISP_AF_MAX),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  d2d_lp0_nr_yth_0, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  d2d_lp0_nr_yth_1, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  d2d_lp0_nr_yth_2, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  d2d_lp0_nr_yth_3, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  d2d_lp0_nr_yth_4, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  d2d_lp0_nr_yth_5, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  d2d_lp0_nr_yth_6, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  d2d_lp0_nr_yth_7, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  d2d_lp0_nr_yth_8, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  d2d_lp0_nr_yth_9, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, d2d_lp0_nr_yth_10, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, d2d_lp0_nr_yth_11, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, d2d_lp0_nr_yth_12, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, d2d_lp0_nr_yth_13, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  d2d_lp1_nr_yth_0, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  d2d_lp1_nr_yth_1, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  d2d_lp1_nr_yth_2, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  d2d_lp1_nr_yth_3, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  d2d_lp1_nr_yth_4, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  d2d_lp1_nr_yth_5, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  d2d_lp1_nr_yth_6, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  d2d_lp1_nr_yth_7, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  d2d_lp1_nr_yth_8, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  d2d_lp1_nr_yth_9, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, d2d_lp1_nr_yth_10, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, d2d_lp1_nr_yth_11, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, d2d_lp1_nr_yth_12, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, d2d_lp1_nr_yth_13, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  d2d_lp2_nr_yth_0, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  d2d_lp2_nr_yth_1, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  d2d_lp2_nr_yth_2, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  d2d_lp2_nr_yth_3, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  d2d_lp2_nr_yth_4, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  d2d_lp2_nr_yth_5, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  d2d_lp2_nr_yth_6, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  d2d_lp2_nr_yth_7, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  d2d_lp2_nr_yth_8, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  d2d_lp2_nr_yth_9, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, d2d_lp2_nr_yth_10, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, d2d_lp2_nr_yth_11, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, d2d_lp2_nr_yth_12, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, d2d_lp2_nr_yth_13, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_0,  d3d_flt0_thr_vc_0, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_1,  d3d_flt0_thr_vc_1, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_2,  d3d_flt0_thr_vc_2, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_3,  d3d_flt0_thr_vc_3, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_4,  d3d_flt0_thr_vc_4, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_5,  d3d_flt0_thr_vc_5, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_6,  d3d_flt0_thr_vc_6, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_7,  d3d_flt0_thr_vc_7, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_8,  d3d_flt0_thr_vc_8, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_9,  d3d_flt0_thr_vc_9, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_10, d3d_flt0_thr_vc_10, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_11, d3d_flt0_thr_vc_11, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_12, d3d_flt0_thr_vc_12, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_cfg_13, d3d_flt0_thr_vc_13, ISP_REG_TBL_LENGTH_32),
};

static struct IspParamAttribute IspTuningParam[] = {
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, flash_gain),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, flash_delay_frame),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, hor_visual_angle),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, ver_visual_angle),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, focus_length),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, gamma_num),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, rolloff_ratio),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, flicker_type),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, flicker_ratio),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, flicker_local_comp),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, afs_sel),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, afs_low_th),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, afs_high_th),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, gtm_hist_sel),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, gtm_type),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, gtm_bright_dn_limit),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, gtm_interval),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, gtm_reserve0),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, gtm_reserve1),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, gtm_reserve2),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, gtm_reserve3),

	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, grad_th),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, dir_v_th),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, dir_h_th),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, res_smth_high),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, res_smth_low),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, res_high_th),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, res_low_th),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, res_dir_a),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, res_dir_d),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, res_dir_v),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, res_dir_h),

	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, fpn_pfpn_en),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, fpn_cfpn_en),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, fpn_pfpn_cluster_size),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, fpn_pfpn_phase_lut, ISP_PFPN_TBL_SIZE),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, fpn_pfpn_period_lut, ISP_PFPN_TBL_SIZE),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, fpn_pfpn_offset_lut, ISP_PFPN_TBL_SIZE),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, fpn_cfpn_lw_th),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, fpn_cfpn_hi_th),

	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, dpc_comm_cfg, ISP_DPC_COMM_MAX),

	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, ctc_low_th_min),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, ctc_low_th),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, ctc_high_th),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, ctc_high_th_max),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, ctc_dir_wt),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, ctc_dir_th),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, bayer_gain, ISP_RAW_CH_MAX),

	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, ff_mod),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, lsc_mode),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, lsc_center_x),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, lsc_center_y),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, lsc_trig_cfg, ISP_LSC_TEMP_NUM),

	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, mff_mod),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, msc_mode),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, msc_trig_cfg, ISP_MSC_TEMP_NUM),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, msc_blw_lut, ISP_MSC_TBL_LUT_SIZE),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, msc_blh_lut, ISP_MSC_TBL_LUT_SIZE),

	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, gamma_trig_cfg, ISP_GAMMA_TRIGGER_POINTS),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, ccm_trig_cfg, ISP_CCM_TEMP_NUM),

	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, lca_comm_cfg, ISP_LCA_COMM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, gca_cfg, ISP_GCA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, pltm_cfg, ISP_PLTM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, sharp_comm_cfg, ISP_SHARP_COMM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, denoise_comm_cfg, ISP_DENOISE_COMM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, tdf_comm_cfg, ISP_TDF_COMM_MAX),
#ifdef USE_ENCPP
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, encpp_top_comm_cfg, ENCPP_TOP_COMM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, encpp_sharp_comm_cfg, ENCPP_SHARP_COMM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, encpp_ldci_comm_cfg, ENCPP_LDCI_COMM_MAX),
#endif
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, nrp_comm_cfg, ISP_NRP_COMM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, sensor_temp, 14 * TEMP_COMP_MAX),

	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_d2d_nr_cbth, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_d2d_nr_crth, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_d2d_lp0_nr_ybk, ISP_REG_TBL_LENGTH_16),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_d2d_lp1_nr_ybk, ISP_REG_TBL_LENGTH_16),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_d2d_lp2_nr_ybk, ISP_REG_TBL_LENGTH_16),

	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_tdnf_df_shape, ISP_REG_TBL_LENGTH_16),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_tdnf_ratio_amp, ISP_REG_TBL_LENGTH_16),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_tdnf_k_dlt_bk, ISP_REG_TBL_LENGTH_16),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_tdnf_ct_rt_bk, ISP_REG_TBL_LENGTH_16),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_tdnf_dtc_hf_bk, ISP_REG_TBL_LENGTH_16),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_tdnf_dtc_mf_bk, ISP_REG_TBL_LENGTH_16),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_tdnf_dtc_hf_ba, ISP_REG_TBL_LENGTH_16),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_tdnf_dtc_mf_ba, ISP_REG_TBL_LENGTH_16),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_tdnf_lay0_d2d0_rt_br, ISP_REG_TBL_LENGTH_16),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_tdnf_lay1_d2d0_rt_br, ISP_REG_TBL_LENGTH_16),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_tdnf_lay0_nrd_rt_br, ISP_REG_TBL_LENGTH_16),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_tdnf_lay1_nrd_rt_br, ISP_REG_TBL_LENGTH_16),

	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, lca_pf_satu_lut, ISP_REG_TBL_LENGTH_33),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, lca_gf_satu_lut, ISP_REG_TBL_LENGTH_33),

	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_sharp_hs_value, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_sharp_ms_value, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_sharp_ls_value, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_sharp_hs_lum, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_sharp_ms_lum, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_sharp_ls_lum, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_sharp_hsv, 46),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, sharp_ratio_by_mot, ISP_SHARP_RT_TBL_SIZE),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, sharp_ratio_by_tex, ISP_SHARP_RT_TBL_SIZE),
#ifdef USE_ENCPP
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, encpp_gbl_satu_adj_lut, ISP_REG_TBL_LENGTH_16),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, encpp_sharp_hs_value, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, encpp_sharp_ms_value, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, encpp_sharp_hs_lum, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, encpp_sharp_ms_lum, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, encpp_sharp_hsv, 46),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, encpp_sharp_ls_map_lut, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, encpp_sharp_texture_lut, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, encpp_sharp_ratio_by_mot, ISP_SHARP_RT_TBL_SIZE),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, encpp_sharp_ratio_by_tex, ISP_SHARP_RT_TBL_SIZE),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, encpp_ldci_txt_ratio_lut, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, encpp_ldci_mot_ratio_lut, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, encpp_ldci_lum_diff_lut, ISP_REG_TBL_LENGTH_32),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, encpp_ldci_map_pos_lut, ISP_REG_TBL_LENGTH_16),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, encpp_ldci_map_neg_lut, ISP_REG_TBL_LENGTH_16),
#endif

	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_wdr_de_purpl_hsv_tbl, ISP_WDR_TBL_SIZE),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_pltm_stat_gd_cv, 15),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_pltm_df_cv, ISP_REG_TBL_LENGTH_33),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_pltm_lum_map_cv, ISP_PLTM_LM_TBL_SIZE),

	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, fpn_cfpn_coeff_lut, ISP_REG_TBL_LENGTH_17),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, fpn_pfpn_scale_coeff_lut, ISP_PFPN_TBL_SIZE),

	ISP_FILE_STRUCT_ARRAY_ATTR(isp_color_matrix0, matrix, 12),
	ISP_FILE_STRUCT_ARRAY_ATTR(isp_color_matrix1, matrix, 12),
	ISP_FILE_STRUCT_ARRAY_ATTR(isp_color_matrix2, matrix, 12),
};

static struct FileAttribute FileAttr[] = {
	{"isp_test_param.ini", array_size(IspTestParam), &IspTestParam[0],},
	{"isp_3a_param.ini", array_size(Isp3aParam), &Isp3aParam[0],},
	{"isp_dynamic_param.ini", array_size(IspDynamicParam), &IspDynamicParam[0],},
	{"isp_tuning_param.ini", array_size(IspTuningParam), &IspTuningParam[0],},
};

#define MAGIC_NUM 0x64387483

int cfg_get_int(dictionary *ini, char *main, char *sub, int *val)
{
	char key[128] = { 0 };
	int ret;

	strcpy(key, main);
	strcat(key,":");
	strcat(key,sub);
	ret = isp_iniparser_getint(ini, key, MAGIC_NUM);
	if(ret == MAGIC_NUM) {
		*val = 0;
		return -1;
	}
	*val = ret;
	return 0;
}

int cfg_get_double(dictionary *ini, char *main, char *sub, double *val)
{
	char key[128] = { 0 };
	double ret;

	strcpy(key, main);
	strcat(key, ":");
	strcat(key, sub);
	ret = isp_iniparser_getdouble(ini, key, MAGIC_NUM/1.0);
	if((ret <= MAGIC_NUM/1.0 + 1.0) || (ret >= MAGIC_NUM/1.0 - 1.0)) {
		*val = 0.0;
		return -1;
	}
	*val = ret;
	//printf("MAGIC_NUM/1.0 = %f, MAGIC_NUM = %d\n", MAGIC_NUM/1.0, MAGIC_NUM);
	return 0;
}

int cfg_get_int_arr(dictionary *ini, char *main, char *sub, int *arr, int len)
{
	char key[128] = { 0 };
	int ret;

	strcpy(key, main);
	strcat(key, ":");
	strcat(key, sub);
	ret = isp_iniparser_get_int_array(ini, key, arr, len, MAGIC_NUM);
	if(ret == MAGIC_NUM) {
		return -1;
	}
	return ret;
}

int cfg_get_double_arr(dictionary *ini, char *main, char *sub, double *arr, int len)
{
	char key[128] = { 0 };
	int ret;

	strcpy(key, main);
	strcat(key, ":");
	strcat(key, sub);
	ret = isp_iniparser_get_double_array(ini, key, arr, len, MAGIC_NUM);
	if(ret == MAGIC_NUM) {
		return -1;
	}
	return ret;
}

int isp_read_file(char *file_path, char *buf, size_t len)
{
	FILE *fp = NULL;
	size_t buf_len;
	struct stat s;

	if (stat(file_path, &s)) {
		ISP_ERR("%s stat error!\n", file_path);
		return -1;
	}

	fp = fopen(file_path, "r");
	if (!fp) {
		ISP_ERR("open %s failed!\n", file_path);
		return -1;
	}

	buf_len = fread(buf, 1, s.st_size, fp);
	ISP_CFG_LOG(ISP_LOG_CFG, "%s len = %d, expect len = %lld\n", file_path, buf_len, s.st_size);
	fclose(fp);

	if (buf_len <= 0)
		return -1;

	return buf_len;
}

int isp_parser_cfg(struct isp_param_config *isp_ini_cfg,
		  dictionary *ini, struct FileAttribute *file_attr)
{
	int i, j, *array_value, val = 0;
	struct IspParamAttribute *param;
	char sub_name[128] = { 0 };

	/* fetch ISP config! */
	for (i = 0; i < file_attr->param_len; i++) {
		param = file_attr->pIspParam + i;
		if (param->main == NULL || param->sub == NULL) {
			ISP_WARN("param->main or param->sub is NULL!\n");
			continue;
		}
		if (param->len == 1) {
			if (-1 == cfg_get_int(ini, param->main, param->sub, &val)) {
				ISP_WARN("fetch %s->%s failed!\n", param->main, param->sub);
			} else {
				if (param->set_param) {
					param->set_param(isp_ini_cfg, (void *)&val, param->len);
					ISP_CFG_LOG(ISP_LOG_CFG, "fetch %s->%s value = %d\n", param->main, param->sub, val);
				}
			}
		} else if (param->len > 1) {
			array_value = (int *)malloc(param->len * sizeof(int));
			memset(array_value, 0, param->len * sizeof(int));

			param->len = cfg_get_int_arr(ini, param->main, param->sub, array_value, param->len);
			if (-1 == param->len) {
				ISP_WARN("fetch %s->%s failed!\n", param->main, param->sub);
			} else {
				if (param->set_param) {
					param->set_param(isp_ini_cfg, (void *)array_value, param->len);
					ISP_CFG_LOG(ISP_LOG_CFG, "fetch %s->%s length = %d\n", param->main, param->sub, param->len);
				}
			}

			if (array_value)
				free(array_value);
		}
	}
	ISP_PRINT("fetch isp_cfg done!\n");
	return 0;
}

int isp_parser_tbl(struct isp_param_config *isp_ini_cfg, char *tbl_patch)
{
	int len;
	char isp_tbl_path[128] = "\0";

	ISP_PRINT("Fetch table from \"%s\"\n", tbl_patch);
	/* fetch pltm_tbl table! */
	strcpy(isp_tbl_path, tbl_patch);
	strcat(isp_tbl_path, "pltm_tbl.bin");
	len = isp_read_file(isp_tbl_path, (char *)isp_ini_cfg->isp_tunning_settings.isp_pltm_gtm_tbl, sizeof(isp_ini_cfg->isp_tunning_settings.isp_pltm_gtm_tbl));
	if (len < 0 || len != sizeof(isp_ini_cfg->isp_tunning_settings.isp_pltm_gtm_tbl)) {
		ISP_WARN("read pltm_tbl from failed! len = %d, but %d is required\n", len, sizeof(isp_ini_cfg->isp_tunning_settings.isp_pltm_gtm_tbl));
	}

	/* fetch fpn_tbl table! */
	strcpy(isp_tbl_path, tbl_patch);
	strcat(isp_tbl_path, "fpn_tbl.bin");
	len = isp_read_file(isp_tbl_path, (char *)isp_ini_cfg->isp_tunning_settings.fpn_cfpn_gain_corr_tbl, ISP_CFPN_GAIN_CORR_TBL_SIZE);
	if (len < 0 || len != ISP_CFPN_GAIN_CORR_TBL_SIZE) {
		ISP_WARN("read fpn_tbl from failed! len = %d, but %d is required\n", len, ISP_CFPN_GAIN_CORR_TBL_SIZE);
	}

	/* fetch dpc_tbl table! */
	strcpy(isp_tbl_path, tbl_patch);
	strcat(isp_tbl_path, "dpc_tbl.bin");
	len = isp_read_file(isp_tbl_path, (char *)isp_ini_cfg->isp_tunning_settings.dpc_static_dead_pix_tbl, sizeof(isp_ini_cfg->isp_tunning_settings.dpc_static_dead_pix_tbl));
	if (len < 0 || len != sizeof(isp_ini_cfg->isp_tunning_settings.dpc_static_dead_pix_tbl)) {
		ISP_WARN("read dpc_tbl from failed! len = %d, but %d is required\n", len, sizeof(isp_ini_cfg->isp_tunning_settings.dpc_static_dead_pix_tbl));
	}

	/* fetch gca_tbl table! */
	strcpy(isp_tbl_path, tbl_patch);
	strcat(isp_tbl_path, "gca_tbl.bin");
	len = isp_read_file(isp_tbl_path, (char *)isp_ini_cfg->isp_tunning_settings.gca_hor_r_offset, 825 * 4);
	if (len < 0 || len != 825 * 4) {
		ISP_WARN("read gca_tbl from failed! len = %d, but %d is required\n", len, 825 * 4);
	}

	/* fetch gamma_tbl table! */
	strcpy(isp_tbl_path, tbl_patch);
	strcat(isp_tbl_path, "gamma_tbl.bin");
	len = isp_read_file(isp_tbl_path, (char *)isp_ini_cfg->isp_tunning_settings.gamma_tbl_ini, SIZE_OF_GAMMA_TBL);
	if (len < 0 || len != SIZE_OF_GAMMA_TBL) {
		ISP_WARN("read gamma_tbl from failed! len = %d, but %d is required\n", len, SIZE_OF_GAMMA_TBL);
		//isp_ini_cfg->isp_test_settings.gamma_en = 0;
	}

	/* fetch lsc table! */
	strcpy(isp_tbl_path, tbl_patch);
	strcat(isp_tbl_path, "lsc_tbl.bin");
	len = isp_read_file(isp_tbl_path, (char *)isp_ini_cfg->isp_tunning_settings.lsc_tbl, SIZE_OF_LSC_TBL);
	if (len < 0 || len != SIZE_OF_LSC_TBL) {
		ISP_WARN("read lsc_tbl from failed! len = %d, but %d is required\n", len, SIZE_OF_LSC_TBL);
		//isp_ini_cfg->isp_test_settings.lsc_en = 0;
	}

	/* fetch msc table! */
	strcpy(isp_tbl_path, tbl_patch);
	strcat(isp_tbl_path, "msc_tbl.bin");
	len = isp_read_file(isp_tbl_path, (char *)isp_ini_cfg->isp_tunning_settings.msc_tbl, sizeof(isp_ini_cfg->isp_tunning_settings.msc_tbl));
	if (len < 0 || len != sizeof(isp_ini_cfg->isp_tunning_settings.msc_tbl)) {
		ISP_WARN("read msc_tbl from failed! len = %d, but %d is required\n", len, (unsigned int)sizeof(isp_ini_cfg->isp_tunning_settings.msc_tbl));
		//isp_ini_cfg->isp_test_settings.msc_en = 0;
	}

	/* fetch cem table! */
	strcpy(isp_tbl_path, tbl_patch);
	strcat(isp_tbl_path, "cem_tbl.bin");
	len = isp_read_file(isp_tbl_path, (char *)isp_ini_cfg->isp_tunning_settings.isp_cem_table, ISP_CEM_MEM_SIZE + ISP_CEM_MEM_SIZE);
	if (len < 0 || len != ISP_CEM_MEM_SIZE + ISP_CEM_MEM_SIZE) {
		ISP_WARN("read cem_tbl from failed! len = %d, but %d is required\n", len, ISP_CEM_MEM_SIZE + ISP_CEM_MEM_SIZE);
		//isp_ini_cfg->isp_test_settings.cem_en = 0;
	}

	return 0;
}
#endif

struct isp_cfg_array cfg_arr[] = {
#if (ISP_VERSION == 610)
#if defined(SENSOR_CV2003) || defined(SENSOR_SC231HAI) || defined(SENSOR_SC235HAI) || defined(SENSOR_GC2083) || \
	defined(SENSOR_GC4663) || defined(SENSOR_SC485SL) || defined(SENSOR_SC635HAI) ||  defined(SENSOR_GC8613) || \
	defined(SENSOR_SC2337P) || defined(SENSOR_CV8002) || defined(SENSOR_SC533HAI) || defined(SENSOR_CV2005)  || \
	defined(SENSOR_IMX681) || defined(SENSOR_IMX766) || defined(SENSOR_CV5003) || defined(SENSOR_GC13A2) || defined(SENSOR_OV13850)
#ifdef SENSOR_CV2003
	{"cv2003_mipi",  "cv2003_mipi_isp610_20260109_103158_day", 1920, 1080, 15, 0, 0, &cv2003_mipi_isp_cfg},
	{"cv2003_mipi",  "cv2003_mipi_isp610_20260109_103158_ir", 1920, 1080, 15, 0, 1, &cv2003_mipi_ir_isp_cfg},
	{"cv2003_mipi_2",  "cv2003_mipi_isp610_20260109_103158_day", 1920, 1080, 15, 0, 0, &cv2003_mipi_isp_cfg},
	{"cv2003_mipi_2",  "cv2003_mipi_isp610_20260109_103158_ir", 1920, 1080, 15, 0, 1, &cv2003_mipi_ir_isp_cfg},
	{"cv2003_mipi_3",  "cv2003_mipi_isp610_20260109_103158_day", 1920, 1080, 15, 0, 0, &cv2003_mipi_isp_cfg},
	{"cv2003_mipi_3",  "cv2003_mipi_isp610_20260109_103158_ir", 1920, 1080, 15, 0, 1, &cv2003_mipi_ir_isp_cfg},
#endif

#ifdef SENSOR_SC231HAI
	{"sc231hai_mipi",  "sc231hai_mipi_default_ini_isp610", 1920, 1080, 30, 0, 0, &sc231hai_mipi_isp_cfg},
#endif

#ifdef SENSOR_SC2337P
	{"sc2337p_mipi",  "sc2337p_mipi_default_ini_isp610", 1920, 1080, 30, 0, 0, &sc2337p_mipi_isp_cfg},
	{"sc2337p_mipi",  "sc2337p_mipi_default_ini_ir_isp610", 1920, 1080, 30, 0, 1, &sc2337p_mipi_ir_isp_cfg},
	{"sc2337p_mipi",  "sc2337p_mipi_isp610_20251218_170620_base_aiisp", 1920, 1080, 15, 0, 2, &sc2337p_mipi_ai_isp_cfg},
#endif

#ifdef SENSOR_SC235HAI
	{"sc235hai_mipi",  "sc235hai_mipi_isp610_20260410_103408_day", 1920, 1080, 30, 0, 0, &sc235hai_mipi_isp_cfg},
	{"sc235hai_mipi",  "sc235hai_mipi_isp610_20260109_204129_ir", 1920, 1080, 30, 0, 1, &sc235hai_mipi_ir_isp_cfg},
	{"sc235hai_mipi",  "sc235hai_mipi_isp610_20260416_211412_aiisp", 1920, 1080, 15, 0, 2, &sc235hai_mipi_aiisp_isp_cfg},
#endif

#ifdef SENSOR_OV13850
	{"ov13850_mipi", "ov13850_mipi_isp610_20261021_195907", 2560, 1440, 60, 0, 0, &ov13850_mipi_isp_cfg},
	/* 30 fps uses the same vendor 60 fps tuning; no separate 30 fps calibration was supplied. */
	{"ov13850_mipi", "ov13850_mipi_isp610_20261021_195907", 2560, 1440, 30, 0, 0, &ov13850_mipi_isp_cfg},
	/* Other geometries reuse the supplied 1440p tuning; separate IQ calibration is pending. */
	{"ov13850_mipi", "ov13850_mipi_isp610_20261021_195907", 3840, 2160, 25, 0, 0, &ov13850_mipi_isp_cfg},
	{"ov13850_mipi", "ov13850_mipi_isp610_20261021_195907", 1920, 1080, 30, 0, 0, &ov13850_mipi_isp_cfg},
	{"ov13850_mipi", "ov13850_mipi_isp610_20261021_195907", 1920, 1080, 60, 0, 0, &ov13850_mipi_isp_cfg},
	{"ov13850_mipi", "ov13850_mipi_isp610_20261021_195907", 1280, 720, 30, 0, 0, &ov13850_mipi_isp_cfg},
	{"ov13850_mipi", "ov13850_mipi_isp610_20261021_195907", 1280, 720, 60, 0, 0, &ov13850_mipi_isp_cfg},
#endif

#ifdef SENSOR_GC2083
	{"gc2083_mipi",  "gc2083_mipi_default_ini_isp610", 1920, 1080, 15, 0, 0, &gc2083_mipi_isp_cfg},
#endif

#ifdef SENSOR_GC4663
	{"gc4663_mipi", "gc4663_mipi_default_ini_isp610", 2560, 1440, 15, 0, 0, &gc4663_mipi_isp_cfg},
#endif

#ifdef SENSOR_GC8613
	{"gc8613_mipi", "gc8613_mipi_isp610_20260109_211717_day", 3840, 2160, 15, 0, 0, &gc8613_mipi_isp_cfg},
	{"gc8613_mipi", "gc8613_mipi_isp610_20260109_213204_ir", 3840, 2160, 15, 0, 1, &gc8613_mipi_ir_isp_cfg},
#endif

#ifdef SENSOR_SC485SL
	{"sc485sl_mipi", "sc485sl_mipi_isp610_20260109_172438_day", 2688, 1520, 25, 0, 0, &sc485sl_mipi_isp_cfg},
	{"sc485sl_mipi", "sc485sl_mipi_isp610_20260109_174233_ir", 2688, 1520, 25, 0, 1, &sc485sl_mipi_ir_isp_cfg},
	{"sc485sl_mipi", "sc485sl_mipi_isp610_20251024_192746_aiisp", 2688, 1520, 10, 0, 2, &sc485sl_mipi_ai_isp_cfg},
#endif

#ifdef SENSOR_SC635HAI
	{"sc635hai_mipi", "sc635hai_mipi_default_ini_isp610", 3200, 1800, 30, 0, 0, &sc635hai_mipi_isp_cfg},
#endif

#ifdef SENSOR_CV8002
	{"cv8002_mipi", "cv8002_mipi_isp610_20251226_202226", 3840, 2160, 15, 0, 0, &cv8002_mipi_isp_cfg},
	{"cv8002_mipi", "cv8002_mipi_isp610_20251014_202353_ir", 3840, 2160, 15, 0, 1, &cv8002_mipi_ir_isp_cfg},
#endif

#ifdef SENSOR_SC533HAI
	{"sc533hai_mipi", "sc533hai_mipi_isp610_20260116_092500_v12", 2880, 1620, 25, 0, 0, &sc533hai_mipi_isp_cfg},
	{"sc533hai_mipi", "sc533hai_mipi_isp610_20260512_212640_v14_wdr", 2880, 1620, 25, 1, 0, &sc533hai_mipi_wdr_isp_cfg},
#endif

#ifdef SENSOR_CV2005
	{"cv2005_mipi", "cv2005_mipi_isp610_20251202_170130", 1920, 1080, 15, 0, 0, &cv2005_mipi_isp_cfg},
#endif

#ifdef SENSOR_CV5003
	{"cv5003_mipi", "cv5003_mipi_isp610_20260417_145901_fullsize", 2880, 1620, 30, 0, 0, &cv5003_mipi_isp_cfg},
	{"cv5003_mipi", "cv5003_mipi_isp610_20260417_145901_eis", 2688, 1520, 30, 0, 0, &cv5003_mipi_eis_isp_cfg},
#endif

#ifdef SENSOR_IMX681
	{"imx681_mipi", "imx681_mipi_isp610_20251023_150150uvcp", 4032, 3024, 10, 0, 0, &imx681_mipi_isp_cfg},
#endif

#ifdef SENSOR_IMX766
	{"imx766_mipi", "imx766_mipi_default_pdaf", 3264, 2448, 29, 0, 0, &imx766_mipi_isp_cfg},
#endif

#ifdef SENSOR_GC13A2
	{"gc13a2_mipi", "gc13a2_mipi_default_ini_isp610_pdaf", 4208, 3120, 15, 0, 0, &gc13a2_mipi_isp_cfg},
#endif

#else
	{"gc4663_mipi", "gc4663_mipi_default_ini_isp610", 2560, 1440, 15, 0, 0, &gc4663_mipi_isp_cfg},
#endif
#else
	{"gc4663_mipi", "gc4663_mipi_default_ini_isp610", 2560, 1440, 15, 0, 0, &gc4663_mipi_isp_cfg},
#endif
};

int find_nearest_index(int mod, int temp)
{
	int i = 0, index = 0;
	int min_dist = 1 << 30, tmp_dist;
	int temp_array1[4] = {2800, 4000, 5000, 6500};
	int temp_array2[6] = {2200, 2800, 4000, 5000, 5500, 6500};

	if(mod == 1) {
		for(i = 0; i < 4; i++) {
			tmp_dist = temp_array1[i] - temp;
			tmp_dist = (tmp_dist < 0) ? -tmp_dist : tmp_dist;
			if(tmp_dist < min_dist) {
				min_dist = tmp_dist;
				index = i;
			}
			ISP_CFG_LOG(ISP_LOG_CFG, "mode: %d, tmp_dist: %d, min_dist: %d, index: %d.\n", mod, tmp_dist, min_dist, index);
		}
	} else if (mod == 2) {
		for(i = 0; i < 6; i++) {
			tmp_dist = temp_array2[i] - temp;
			tmp_dist = (tmp_dist < 0) ? -tmp_dist : tmp_dist;
			if(tmp_dist < min_dist) {
				min_dist = tmp_dist;
				index = i;
			}
			ISP_CFG_LOG(ISP_LOG_CFG, "mode: %d, tmp_dist: %d, min_dist: %d, index: %d.\n", mod, tmp_dist, min_dist, index);
		}
	} else {
		ISP_ERR("mod error.\n");
	}
	ISP_CFG_LOG(ISP_LOG_CFG, "nearest temp index: %d.\n", index);
	return index;
}

int isp_save_tbl(struct isp_param_config *param, char *tbl_patch)
{
	FILE *file_fd = NULL;
	char fdstr[50];

	sprintf(fdstr, "%sgamma_tbl.bin", tbl_patch);
	file_fd = fopen(fdstr, "wb");
	if (!file_fd) {
		ISP_WARN("open %s failed!!!\n", fdstr);
		return -1;
	} else {
		fwrite(param->isp_tunning_settings.gamma_tbl_ini, SIZE_OF_GAMMA_TBL, 1, file_fd);
		ISP_PRINT("save isp_ctx to %s success!!!\n", fdstr);
	}
	fclose(file_fd);

	sprintf(fdstr, "%slsc_tbl.bin", tbl_patch);
	file_fd = fopen(fdstr, "wb");
	if (!file_fd) {
		ISP_WARN("open %s failed!!!\n", fdstr);
		return -1;
	} else {
		fwrite(param->isp_tunning_settings.lsc_tbl, SIZE_OF_LSC_TBL, 1, file_fd);
		ISP_PRINT("save isp_ctx to %s success!!!\n", fdstr);
	}
	fclose(file_fd);

	sprintf(fdstr, "%scem_tbl.bin", tbl_patch);
	file_fd = fopen(fdstr, "wb");
	if (!file_fd) {
		ISP_WARN("open %s failed!!!\n", fdstr);
		return -1;
	} else {
		fwrite(param->isp_tunning_settings.isp_cem_table, ISP_CEM_MEM_SIZE, 1, file_fd);
		ISP_PRINT("save isp_ctx to %s success!!!\n", fdstr);
	}
	fclose(file_fd);

	sprintf(fdstr, "%scem_tbl.bin", tbl_patch);
	file_fd = fopen(fdstr, "wb");
	if (!file_fd) {
		ISP_WARN("open %s failed!!!\n", fdstr);
		return -1;
	} else {
		fwrite(param->isp_tunning_settings.isp_cem_table1, ISP_CEM_MEM_SIZE, 1, file_fd);
		ISP_PRINT("save isp_ctx to %s success!!!\n", fdstr);
	}
	fclose(file_fd);

	return 0;
}

#if ISP_LIB_USE_INIPARSER
int isp_load_ini_param(struct isp_param_config *param, char *path)
{
	int i;
	char isp_tbl_path[ISP_CFG_BIN_PATH_LEN], file_name[ISP_CFG_BIN_PATH_LEN];
	dictionary *ini;

	sprintf(isp_tbl_path, "%s/bin/", path);

	if (access(path, F_OK) == 0) {
		ISP_PRINT("find %s, read ini start!!!\n", path);
		for (i = 0; i < array_size(FileAttr); i++) {
			sprintf(file_name, "%s%s", path, FileAttr[i].file_name);
			ISP_PRINT("Fetch ini file form \"%s\"\n", file_name);

			ini = isp_iniparser_load(file_name);
			if (ini == NULL) {
				ISP_ERR("read ini error!!!\n");
				return -1;
			}
			isp_parser_cfg(param, ini, &FileAttr[i]);
			isp_iniparser_freedict(ini);
		}

		isp_parser_tbl(param, isp_tbl_path);

		ISP_PRINT("read ini end!!!\n");
	} else {
		return -1;
	}

	return 0;
}
#endif

int isp_load_bin_param(struct isp_param_config *param, char *isp_cfg_name, char *path)
{
	char fdstr[ISP_CFG_BIN_PATH_LEN], time[ISP_CFG_BIN_TIME_LEN], notes[ISP_CFG_BIN_NOTES_LEN];
	unsigned int size;
	FILE *file_fd = NULL;

	sprintf(fdstr, "%s/isp_param_config.bin", path);
	file_fd = fopen(fdstr, "r");
	if (!file_fd) {
		return -1;
	} else {
		ISP_PRINT("find %s, load bin start.\n", fdstr);
		fread(&size, sizeof(unsigned int), 1, file_fd);
		if (size != sizeof(struct isp_param_config)) {
			ISP_ERR("%s -- read size %d != isp_param size %d!\n", fdstr, size, (unsigned int)sizeof(struct isp_param_config));
			fclose(file_fd);
			return -1;
		}
		fread(time, sizeof(time), 1, file_fd);
		fread(notes, sizeof(notes), 1, file_fd);
		if (param) {
			fread(param, sizeof(struct isp_param_config), 1, file_fd);
		} else {
			ISP_ERR("param is NULL!\n");
			fclose(file_fd);
			return -1;
		}
		/* update isp_cfg_name for isp_debug_info */
		strcpy(isp_cfg_name, notes);
		ISP_PRINT("Read seccess... Time:%s  Notes:%s\n", time, notes);
	}
	fclose(file_fd);
	return 0;
}

int isp_save_bin(struct isp_param_config *param, char *path)
{
	char time[20] = "2022.12.31";
	char notes[50] = "gc4663_color";
	unsigned int size = sizeof(struct isp_param_config);
	char fdstr[50];
	FILE *file_fd = NULL;

	sprintf(fdstr, "%sisp_param_config.bin", path);
	file_fd = fopen(fdstr, "wb");
	if (!file_fd) {
		ISP_WARN("open %s failed!!!\n", fdstr);
		return -1;
	} else {
		fwrite(&size, sizeof(unsigned int), 1, file_fd);
		fwrite(time, sizeof(time), 1, file_fd);
		fwrite(notes, sizeof(notes), 1, file_fd);
		fwrite(param, sizeof(struct isp_param_config), 1, file_fd);
		ISP_PRINT("save isp_param_config to %s success!!!....  size = %d\n", fdstr, size);
	}
	fclose(file_fd);
	return 0;
}

int isp_load_bsdiff_patch(struct isp_param_config *param, char *isp_cfg_name, char *path)
{
	char fdstr[ISP_CFG_BIN_PATH_LEN], time[ISP_CFG_BIN_TIME_LEN], notes[ISP_CFG_BIN_NOTES_LEN];
	unsigned int i, new_size, old_size = sizeof(struct isp_param_config), adler32_checksum, patch_size;
	unsigned char *patch_data = NULL;
	struct isp_param_config *old_data = NULL;
	FILE *file_fd = NULL;

	sprintf(fdstr, "%sisp_param_config.bsdiff", path);
	file_fd = fopen(fdstr, "r");
	if (!file_fd) {
		return -1;
	} else {
		ISP_PRINT("find %s, load bsdiff start.\n", fdstr);
		fread(&new_size, 1, sizeof(unsigned int), file_fd);//[size]
		if (old_size != new_size) {
			ISP_ERR("old_size = %d, new_size = %d. no match\n", old_size, new_size);
			fclose(file_fd);
			return -1;
		}
		fread(time, 1, sizeof(time), file_fd);//[time]
		fread(notes, 1, sizeof(notes), file_fd);//[notes]
		fread(&adler32_checksum, 1, sizeof(unsigned int), file_fd);//[adler32_checksum]
		old_data = (struct isp_param_config *)malloc(old_size);
		if (!old_data) {
			ISP_ERR("old_data malloc failed. size = %d\n", old_size);
			fclose(file_fd);
			return -1;
		}
		for (i = 0; i < array_size(cfg_arr); i++) {
			old_data->isp_test_settings = *cfg_arr[i].cfg->isp_test_settings;
			old_data->isp_3a_settings = *cfg_arr[i].cfg->isp_3a_settings;
			old_data->isp_iso_settings = *cfg_arr[i].cfg->isp_iso_settings;
			old_data->isp_tunning_settings = *cfg_arr[i].cfg->isp_tunning_settings;
			if (adler32_checksum == adler32((unsigned char *)old_data, old_size))
				break;
		}
		if (i == array_size(cfg_arr)) {
			ISP_ERR("adler32_checksum = %d, no match\n", adler32_checksum);
			free(old_data);
			fclose(file_fd);
			return -1;
		}
		ISP_PRINT("find base config %s_%d_%d_%d_%d_%d -> [%s]\n", cfg_arr[i].sensor_name, cfg_arr[i].width, cfg_arr[i].height,
					cfg_arr[i].fps, cfg_arr[i].wdr, cfg_arr[i].ir, cfg_arr[i].isp_cfg_name);
		fread(&patch_size, 1, sizeof(unsigned int), file_fd);//[patch size]
		patch_data = (unsigned char *)malloc(patch_size);
		if (!patch_data) {
			ISP_ERR("patch_data malloc failed. size = %d\n", patch_size);
			free(old_data);
			fclose(file_fd);
			return -1;
		}
		fread(patch_data, 1, patch_size, file_fd);//[patch data]
		if (isp_bspatch((unsigned char *)old_data, old_size, patch_data, patch_size, (unsigned char *)param, new_size) < 0) {
			ISP_ERR("bspatch failed!\n");
			free(patch_data);
			free(old_data);
			fclose(file_fd);
			return -1;
		}
		free(patch_data);
		free(old_data);
		/* update isp_cfg_name for isp_debug_info */
		strcpy(isp_cfg_name, notes);
		ISP_PRINT("Read bsdiff seccess... Time:%s  Notes:%s\n", time, notes);
	}

	fclose(file_fd);
	return 0;
}

#if 0
int isp_save_bsdiff(char *path)
{
	char time[20] = "2026.3.30";
	char notes[50] = "sc485sl_color";
	struct isp_cfg_pt *old_cfg = &gc4663_mipi_isp_cfg;
	struct isp_cfg_pt *new_cfg = &sc485sl_mipi_isp_cfg;
	unsigned int size, adler32_checksum, patch_size;
	int ret;
	unsigned char *patch_data = NULL;
	struct isp_param_config *old_data = NULL, *new_data = NULL;
	char fdstr[ISP_CFG_BIN_PATH_LEN];
	FILE *file_fd = NULL;

	size = sizeof(struct isp_param_config);
	old_data = (struct isp_param_config *)malloc(sizeof(struct isp_param_config));
	if (!old_data) {
		ISP_ERR("old_data malloc failed. size = %d\n", sizeof(struct isp_param_config));
		return -1;
	}
	new_data = (struct isp_param_config *)malloc(sizeof(struct isp_param_config));
	if (!new_data) {
		ISP_ERR("new_data malloc failed. size = %d\n", sizeof(struct isp_param_config));
		free(old_data);
		return -1;
	}
	old_data->isp_test_settings = *old_cfg->isp_test_settings;
	old_data->isp_3a_settings = *old_cfg->isp_3a_settings;
	old_data->isp_iso_settings = *old_cfg->isp_iso_settings;
	old_data->isp_tunning_settings = *old_cfg->isp_tunning_settings;
	new_data->isp_test_settings = *new_cfg->isp_test_settings;
	new_data->isp_3a_settings = *new_cfg->isp_3a_settings;
	new_data->isp_iso_settings = *new_cfg->isp_iso_settings;
	new_data->isp_tunning_settings = *new_cfg->isp_tunning_settings;

	adler32_checksum = adler32((unsigned char *)old_data, size);

	//bsdiff
	ret = isp_bsdiff((unsigned char *)old_data, size, (unsigned char *)new_data, size, &patch_data);
	if (ret < 0) {
		ISP_ERR("bsdiff failed!\n");
		free(new_data);
		free(old_data);
		return -1;
	}
	patch_size = ret;
	ISP_PRINT("bsdiff generated successfully, patch_size = %d!\n", patch_size);

	//write bsdiff file
	sprintf(fdstr, "%sisp_param_config.bsdiff", path);
	file_fd = fopen(fdstr, "wb");
	if (!file_fd) {
		ISP_ERR("open %s failed.\n", fdstr);
		free(patch_data);
		free(new_data);
		free(old_data);
		return -1;
	} else {
		fwrite(&size, 1, sizeof(unsigned int), file_fd);//[size]
		fwrite(time, 1, sizeof(time), file_fd);//[time]
		fwrite(notes, 1, sizeof(notes), file_fd);//[notes]
		fwrite(&adler32_checksum, 1, sizeof(unsigned int), file_fd);//[adler32 checksum]
		fwrite(&patch_size, 1, sizeof(unsigned int), file_fd);//[patch size]
		fwrite(patch_data, 1, patch_size, file_fd);//[patch data]
		fclose(file_fd);
		file_fd = NULL;
		ISP_PRINT("save isp_param_config.bsdiff to %s success!!!....  time->[%s]  notes->[%s]\n", fdstr, time, notes);
	}

	free(patch_data);
	free(new_data);
	free(old_data);
	return 0;
}
#endif

int parse_isp_cfg(struct isp_param_config *param, char *isp_cfg_name, char *sensor_name, int isp_id, int ir, char *isp_cfg_path)
{
	char isp_ini_path[ISP_CFG_BIN_PATH_LEN];

#if ISP_LIB_USE_INIPARSER
	//load ini parameter
	sprintf(isp_ini_path, "%s%s/", isp_cfg_path, sensor_name);
	if (!isp_load_ini_param(param, isp_ini_path))
		return 0;
#endif

	//load bin parameter
	if (!isp_load_bin_param(param, isp_cfg_name, isp_cfg_path))
		return 0;

	//load bsdiff patch
	if (!isp_load_bsdiff_patch(param, isp_cfg_name, isp_cfg_path))
		return 0;

	return -1;
}

int parser_ini_info(struct isp_param_config *param, char *isp_cfg_name, char *sensor_name,
			int w, int h, int fps, int wdr, int ir, int sync_mode, int isp_id)
{
	int i;
	struct isp_cfg_pt *cfg = NULL;
	char isp_cfg_path[ISP_CFG_BIN_PATH_LEN];

#if ISP_LIB_USE_INIPARSER
	if (ir == 0) {
		sprintf(isp_cfg_path, "%s", EXTERNEL_BASE_PATH_COLOR);
	} else if (ir == 1) {
		sprintf(isp_cfg_path, "%s", EXTERNEL_BASE_PATH_IR);
	} else {
		sprintf(isp_cfg_path, "%s", EXTERNEL_BASE_PATH_AIISP);
	}

	if(access(isp_cfg_path, F_OK) == 0) {
		ISP_PRINT("detect %s success.\n", isp_cfg_path);
		//load gz/ini/bin parameter
		if (!parse_isp_cfg(param, isp_cfg_name, sensor_name, isp_id, ir, isp_cfg_path))
			return 0;
	}
#endif

	//load header parameter
	ISP_PRINT("prefer isp config: [%s], %dx%d, %d, %d, %d\n", sensor_name, w, h, fps, wdr, ir);
	for (i = 0; i < array_size(cfg_arr); i++) {
		if (!strncmp(sensor_name, cfg_arr[i].sensor_name, 20) &&
			(w == cfg_arr[i].width) && (h == cfg_arr[i].height) &&
			(fps == cfg_arr[i].fps) && (wdr == cfg_arr[i].wdr) &&
			(ir == cfg_arr[i].ir)) {
				cfg = cfg_arr[i].cfg;
				ISP_PRINT("find %s_%d_%d_%d_%d [%s] isp config\n", cfg_arr[i].sensor_name,
				cfg_arr[i].width, cfg_arr[i].height, cfg_arr[i].fps, cfg_arr[i].wdr, cfg_arr[i].isp_cfg_name);
				break;
			}
	}

	if (i == array_size(cfg_arr)) {
		for (i = 0; i < array_size(cfg_arr); i++) {
			if (!strncmp(sensor_name, cfg_arr[i].sensor_name, 20) && (wdr == cfg_arr[i].wdr)) {
				cfg = cfg_arr[i].cfg;
				ISP_WARN("cannot find %s_%d_%d_%d_%d_%d isp config, use %s_%d_%d_%d_%d_%d -> [%s]\n", sensor_name, w, h, fps, wdr, ir,
					cfg_arr[i].sensor_name,	cfg_arr[i].width, cfg_arr[i].height, cfg_arr[i].fps, cfg_arr[i].wdr,
					cfg_arr[i].ir, cfg_arr[i].isp_cfg_name);
				break;
			}
		}

		if (i == array_size(cfg_arr)) {
			for (i = 0; i < array_size(cfg_arr); i++) {
				if (wdr == cfg_arr[i].wdr) {
					cfg = cfg_arr[i].cfg;
					ISP_WARN("cannot find %s_%d_%d_%d_%d_%d isp config, use %s_%d_%d_%d_%d_%d -> [%s]\n", sensor_name, w, h, fps, wdr, ir,
						cfg_arr[i].sensor_name,	cfg_arr[i].width, cfg_arr[i].height, cfg_arr[i].fps, cfg_arr[i].wdr,
						cfg_arr[i].ir, cfg_arr[i].isp_cfg_name);
					break;
				}
			}
		}

		if (i == array_size(cfg_arr)) {
			ISP_WARN("cannot find %s_%d_%d_%d_%d_%d isp config, use default config [%s]\n",
			sensor_name, w, h, fps, wdr, ir, cfg_arr[i-1].isp_cfg_name);
			cfg = cfg_arr[i-1].cfg;// use the last one
		}
	}
	strcpy(isp_cfg_name, cfg_arr[i].isp_cfg_name);
	if (cfg) {
		param->isp_test_settings = *cfg->isp_test_settings;
		param->isp_3a_settings = *cfg->isp_3a_settings;
		param->isp_iso_settings = *cfg->isp_iso_settings;
		param->isp_tunning_settings = *cfg->isp_tunning_settings;
	} else {
		ISP_ERR("isp%d cfg is NULL!\n", isp_id);
		return -1;
	}

	//isp_save_bsdiff("/tmp/");
	return 0;
}


