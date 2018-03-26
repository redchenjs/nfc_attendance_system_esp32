/*
 * gui_task.c
 *
 *  Created on: 2018-02-13 22:57
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "gfx.h"
#include "esp_log.h"

#include "system/event.h"
#include "tasks/gui_task.h"

#define TAG "gui_task"

static const uint8_t *img_file_ptr[][2] = {
#if defined(CONFIG_SCREEN_PANEL_SSD1331)
                                            {ani0_96x64_gif_ptr, ani0_96x64_gif_end}, // "WiFi"
                                            {ani1_96x64_gif_ptr, ani1_96x64_gif_end}, // "Loading"
                                            {ani2_96x64_gif_ptr, ani2_96x64_gif_end}, // "Success"
                                            {ani3_96x64_gif_ptr, ani3_96x64_gif_end}, // "NFC"
                                            {ani4_96x64_gif_ptr, ani4_96x64_gif_end}, // "PowerOff"
                                            {ani5_96x64_gif_ptr, ani5_96x64_gif_end}, // "Clock"
                                            {ani6_96x64_gif_ptr, ani6_96x64_gif_end}, // "Error"
                                            {ani7_96x64_gif_ptr, ani7_96x64_gif_end}  // "Fail"
#elif defined(CONFIG_SCREEN_PANEL_ST7735)
                                            {ani0_160x80_gif_ptr, ani0_160x80_gif_end}, // "WiFi"
                                            {ani1_160x80_gif_ptr, ani1_160x80_gif_end}, // "Loading"
                                            {ani2_160x80_gif_ptr, ani2_160x80_gif_end}, // "Success"
                                            {ani3_160x80_gif_ptr, ani3_160x80_gif_end}, // "NFC"
                                            {ani4_160x80_gif_ptr, ani4_160x80_gif_end}, // "PowerOff"
                                            {ani5_160x80_gif_ptr, ani5_160x80_gif_end}, // "Clock"
                                            {ani6_160x80_gif_ptr, ani6_160x80_gif_end}, // "Error"
                                            {ani7_160x80_gif_ptr, ani7_160x80_gif_end}  // "Fail"
#endif
                                        };
uint8_t img_file_index = 0;

void gui_show_image(uint8_t filename_index)
{
#if defined(CONFIG_ENABLE_GUI)
    if (filename_index >= (sizeof(img_file_ptr) / 2)) {
        ESP_LOGE(TAG, "invalid filename index");
        return;
    }
    img_file_index = filename_index;
    xEventGroupSetBits(task_event_group, GUI_RELOAD_BIT);
#endif
}

void gui_task(void *pvParameter)
{
    gfxInit();

    while (1) {
        gdispImage gfx_image;
        if (!(gdispImageOpenMemory(&gfx_image, img_file_ptr[img_file_index][0]) & GDISP_IMAGE_ERR_UNRECOVERABLE)) {
            gdispImageSetBgColor(&gfx_image, White);
            while (1) {
                if (xEventGroupGetBits(task_event_group) & GUI_RELOAD_BIT) {
                    xEventGroupClearBits(task_event_group, GUI_RELOAD_BIT);
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