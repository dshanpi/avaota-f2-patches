/* GC2093 initial ISP610 overrides. K230 reference, not module calibration.
 * Common processing tables remain in the SDK GC2083 ISP610 baseline.
 * WB ratios: reciprocal of K230 normalized WB gains, Q8. CCM: Q8.
 */
#ifndef GC2093_K230_REFERENCE_ISP610_H
#define GC2093_K230_REFERENCE_ISP610_H

#include "../gc2083/gc2083_mipi_default_ini_isp610.h"

static void gc2093_apply_reference(struct isp_param_config *param)
{
    static const int ae_table[] = {
        32000, 30, 256, 256, 256, 256,
        30, 30, 256, 4096, 256, 256
    };
    static const int light_info[][10] = {
        {269, 256, 87, 256, 256, 256, 45, 2850, 45, 100},
        {250, 256, 90, 256, 256, 256, 45, 3000, 45, 100},
        {199, 256, 113, 256, 256, 256, 45, 4000, 45, 100},
        {184, 256, 132, 256, 256, 256, 45, 5000, 45, 100},
        {164, 256, 149, 256, 256, 256, 45, 6500, 45, 100},
    };
    static const int ccm[3][3][3] = {
        {{300, 54, -98}, {-48, 336, -32}, {-6, -170, 432}},
        {{278, 6, -28}, {-48, 324, -20}, {16, -56, 296}},
        {{270, 6, -20}, {-26, 324, -42}, {12, -2, 246}},
    };
    struct isp_3a_param *ae = &param->isp_3a_settings;
    int i, row, col;

    memcpy(ae->ae_table_preview, ae_table, sizeof(ae_table));
    memcpy(ae->ae_table_capture, ae_table, sizeof(ae_table));
    memcpy(ae->ae_table_video, ae_table, sizeof(ae_table));
    ae->exp_delay_frame = 2;
    ae->gain_delay_frame = 2;
    ae->awb_light_num = 5;
    memset(ae->awb_light_info, 0, sizeof(ae->awb_light_info));
    memcpy(ae->awb_light_info, light_info, sizeof(light_info));
    for (i = 0; i < 3; i++)
        for (row = 0; row < 3; row++)
            for (col = 0; col < 3; col++)
                param->isp_tunning_settings.color_matrix_ini[i].matrix[row][col] = ccm[i][row][col];
    param->isp_tunning_settings.ccm_trig_cfg[0] = 2850;
}

#endif
