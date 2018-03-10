/*
 * oled_display.c
 *
 *  Created on: 2018-02-13 22:57
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "gfx.h"
#include "esp_log.h"

#include "tasks/main_task.h"
#include "tasks/oled_display.h"

#define TAG "oled_display"

static const uint8_t *img_file_ptr[][2] = {
                                            {ani0_gif_ptr, ani0_gif_end}, // "WiFi"
                                            {ani1_gif_ptr, ani1_gif_end}, // "Loading"
                                            {ani2_gif_ptr, ani2_gif_end}, // "Success"
                                            {ani3_gif_ptr, ani3_gif_end}, // "NFC"
                                            {ani4_gif_ptr, ani4_gif_end}, // "PowerOff"
                                            {ani5_gif_ptr, ani5_gif_end}, // "Clock"
                                            {ani6_gif_ptr, ani6_gif_end}, // "Error"
                                            {ani7_gif_ptr, ani7_gif_end}  // "Fail"
                                        };
uint8_t img_file_index = 0;

void oled_display_show_image(uint8_t filename_index)
{
    if (filename_index >= (sizeof(img_file_ptr) / 2)) {
        ESP_LOGE(TAG, "invalid filename index");
        return;
    }
    img_file_index = filename_index;
    xEventGroupSetBits(task_event_group, OLED_DISPLAY_RELOAD_BIT);
}

void oled_display_task(void *pvParameter)
{
    gfxInit();

    while (1) {
        gdispImage gfx_image;
        if (!(gdispImageOpenMemory(&gfx_image, img_file_ptr[img_file_index][0]) & GDISP_IMAGE_ERR_UNRECOVERABLE)) {
            gdispImageSetBgColor(&gfx_image, White);
            while (1) {
                if (xEventGroupGetBits(task_event_group) & OLED_DISPLAY_RELOAD_BIT) {
                    xEventGroupClearBits(task_event_group, OLED_DISPLAY_RELOAD_BIT);
                    break;
                }
                if (gdispImageDraw(&gfx_image, 0, 0, gfx_image.width, gfx_image.height, 0, 0) != GDISP_IMAGE_ERR_OK) {
                    break;
                }
                delaytime_t delay = gdispImageNext(&gfx_image);
                if (delay == TIME_INFINITE) {
                    break;
                }
                if (delay != TIME_IMMEDIATE) {
                    gfxSleepMilliseconds(delay);
                }
            }
            gdispImageClose(&gfx_image);
        } else {
            break;
        }
    }
    ESP_LOGE(TAG, "task failed, rebooting...");
    esp_restart();
}
