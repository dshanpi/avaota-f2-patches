/* Export only the AE adjustment used by this RTSP demonstration.
 * All other values come from the SDK's original OV13850 ISP610 header.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "vendor-reference.h"

static struct isp_param_config params;

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s OUTPUT.bin 25|30\n", argv[0]);
        return 2;
    }
    int fps;
    if (!strcmp(argv[2], "25")) fps = 25;
    else if (!strcmp(argv[2], "30")) fps = 30;
    else return 2;

    params.isp_test_settings = *ov13850_mipi_isp_cfg.isp_test_settings;
    params.isp_3a_settings = *ov13850_mipi_isp_cfg.isp_3a_settings;
    params.isp_tunning_settings = *ov13850_mipi_isp_cfg.isp_tunning_settings;
    params.isp_iso_settings = *ov13850_mipi_isp_cfg.isp_iso_settings;
    if (sizeof(params) != 163984) {
        fprintf(stderr, "Unsupported ISP ABI size: %zu (expected 163984)\n", sizeof(params));
        return 3;
    }

    int table[42] = {32000, fps, 256, 256, 266, 266,
                    fps, fps, 256, 4096, 266, 266};
    params.isp_3a_settings.ae_table_preview_length = 2;
    params.isp_3a_settings.ae_table_capture_length = 2;
    params.isp_3a_settings.ae_table_video_length = 2;
    memcpy(params.isp_3a_settings.ae_table_preview, table, sizeof(table));
    memcpy(params.isp_3a_settings.ae_table_capture, table, sizeof(table));
    memcpy(params.isp_3a_settings.ae_table_video, table, sizeof(table));

    uint32_t size = sizeof(params);
    char date[20] = "2026.11.11", name[50] = {0};
    snprintf(name, sizeof(name), "ov13850_display_%d_ae1_sharp0", fps);
    FILE *out = fopen(argv[1], "wb");
    if (!out) { perror("fopen"); return 4; }
    int failed = fwrite(&size, 4, 1, out) != 1 ||
                 fwrite(date, 20, 1, out) != 1 ||
                 fwrite(name, 50, 1, out) != 1 ||
                 fwrite(&params, size, 1, out) != 1;
    if (fclose(out)) failed = 1;
    if (failed) return 5;
    printf("%s: %u bytes, %s\n", argv[1], size + 74, name);
    return 0;
}
