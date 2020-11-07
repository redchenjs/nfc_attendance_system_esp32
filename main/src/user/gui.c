/*
 * gui.c
 *
 *  Created on: 2018-02-13 22:57
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"

#include "gfx.h"

#include "core/os.h"
#include "user/gui.h"

#define TAG "gui"

static const char *img_file_ptr[][2] = {
#ifdef CONFIG_LCD_TYPE_ST7735
    [GUI_MODE_IDX_GIF_WIFI] = {ani0_160x80_gif_ptr, ani0_160x80_gif_end},
    [GUI_MODE_IDX_GIF_SCAN] = {ani1_160x80_gif_ptr, ani1_160x80_gif_end},
    [GUI_MODE_IDX_GIF_BUSY] = {ani2_160x80_gif_ptr, ani2_160x80_gif_end},
    [GUI_MODE_IDX_GIF_DONE] = {ani3_160x80_gif_ptr, ani3_160x80_gif_end},
    [GUI_MODE_IDX_GIF_FAIL] = {ani4_160x80_gif_ptr, ani4_160x80_gif_end},
    [GUI_MODE_IDX_GIF_PWR]  = {ani5_160x80_gif_ptr, ani5_160x80_gif_end},
    [GUI_MODE_IDX_GIF_CLK]  = {ani6_160x80_gif_ptr, ani6_160x80_gif_end},
    [GUI_MODE_IDX_GIF_CFG]  = {ani7_160x80_gif_ptr, ani7_160x80_gif_end},
    [GUI_MODE_IDX_GIF_UPD]  = {ani8_160x80_gif_ptr, ani8_160x80_gif_end}
#else
    [GUI_MODE_IDX_GIF_WIFI] = {ani0_240x135_gif_ptr, ani0_240x135_gif_end},
    [GUI_MODE_IDX_GIF_SCAN] = {ani1_240x135_gif_ptr, ani1_240x135_gif_end},
    [GUI_MODE_IDX_GIF_BUSY] = {ani2_240x135_gif_ptr, ani2_240x135_gif_end},
    [GUI_MODE_IDX_GIF_DONE] = {ani3_240x135_gif_ptr, ani3_240x135_gif_end},
    [GUI_MODE_IDX_GIF_FAIL] = {ani4_240x135_gif_ptr, ani4_240x135_gif_end},
    [GUI_MODE_IDX_GIF_PWR]  = {ani5_240x135_gif_ptr, ani5_240x135_gif_end},
    [GUI_MODE_IDX_GIF_CLK]  = {ani6_240x135_gif_ptr, ani6_240x135_gif_end},
    [GUI_MODE_IDX_GIF_CFG]  = {ani7_240x135_gif_ptr, ani7_240x135_gif_end},
    [GUI_MODE_IDX_GIF_UPD]  = {ani8_240x135_gif_ptr, ani8_240x135_gif_end}
#endif
};

GDisplay *gui_gdisp = NULL;

static uint8_t gui_backlight = 255;

static coord_t gui_disp_width = 0;
static coord_t gui_disp_height = 0;

static GTimer gui_flush_timer;

static gui_mode_t gui_mode = GUI_MODE_IDX_GIF_WIFI;

static void gui_flush_task(void *pvParameter)
{
    gdispGFlush(gui_gdisp);
}

static void gui_task(void *pvParameter)
{
    portTickType xLastWakeTime;

    gfxInit();

    gui_gdisp = gdispGetDisplay(0);
    gui_disp_width = gdispGGetWidth(gui_gdisp);
    gui_disp_height = gdispGGetHeight(gui_gdisp);

    gtimerStart(&gui_flush_timer, gui_flush_task, NULL, TRUE, TIME_INFINITE);

    ESP_LOGI(TAG, "started.");

#ifdef CONFIG_ENABLE_GUI
    gdispGSetOrientation(gui_gdisp, CONFIG_LCD_ROTATION_DEGREE);
#endif

    while (1) {
        switch (gui_mode) {
        case GUI_MODE_IDX_GIF_WIFI:
        case GUI_MODE_IDX_GIF_SCAN:
        case GUI_MODE_IDX_GIF_BUSY:
        case GUI_MODE_IDX_GIF_DONE:
        case GUI_MODE_IDX_GIF_FAIL:
        case GUI_MODE_IDX_GIF_PWR:
        case GUI_MODE_IDX_GIF_CLK:
        case GUI_MODE_IDX_GIF_CFG:
        case GUI_MODE_IDX_GIF_UPD: {
            gdispImage gfx_image;

            if (!(gdispImageOpenMemory(&gfx_image, img_file_ptr[gui_mode][0]) & GDISP_IMAGE_ERR_UNRECOVERABLE)) {
                gdispImageSetBgColor(&gfx_image, Black);

                gdispGSetBacklight(gui_gdisp, gui_backlight);

                while (1) {
                    xLastWakeTime = xTaskGetTickCount();

                    if (xEventGroupGetBits(user_event_group) & GUI_RLD_MODE_BIT) {
                        xEventGroupClearBits(user_event_group, GUI_RLD_MODE_BIT);
                        break;
                    }

                    if (gdispImageDraw(&gfx_image, 0, 0, gfx_image.width, gfx_image.height, 0, 0) != GDISP_IMAGE_ERR_OK) {
                        ESP_LOGE(TAG, "failed to draw image: %u", gui_mode);
                        gui_mode = GUI_MODE_IDX_OFF;
                        break;
                    }

                    gtimerJab(&gui_flush_timer);

                    delaytime_t delay = gdispImageNext(&gfx_image);
                    if (delay == TIME_INFINITE) {
                        gui_mode = GUI_MODE_IDX_PAUSE;
                        break;
                    }

                    if (delay != TIME_IMMEDIATE) {
                        vTaskDelayUntil(&xLastWakeTime, delay / portTICK_RATE_MS);
                    }
                }

                gdispImageClose(&gfx_image);
            } else {
                ESP_LOGE(TAG, "failed to open image: %u", gui_mode);
                gui_mode = GUI_MODE_IDX_OFF;
                break;
            }
            break;
        }
        case GUI_MODE_IDX_PAUSE:
            xEventGroupWaitBits(
                user_event_group,
                GUI_RLD_MODE_BIT,
                pdTRUE,
                pdFALSE,
                portMAX_DELAY
            );

            break;
        case GUI_MODE_IDX_OFF:
        default:
            gdispGSetBacklight(gui_gdisp, 0);

            vTaskDelay(500 / portTICK_RATE_MS);

            gdispGClear(gui_gdisp, Black);
            gtimerJab(&gui_flush_timer);

            xEventGroupWaitBits(
                user_event_group,
                GUI_RLD_MODE_BIT,
                pdTRUE,
                pdFALSE,
                portMAX_DELAY
            );

            break;
        }
    }
}

void gui_set_mode(gui_mode_t idx)
{
    gui_mode = idx;

    xEventGroupSetBits(user_event_group, GUI_RLD_MODE_BIT);

    ESP_LOGI(TAG, "mode: 0x%02X", gui_mode);
}

gui_mode_t gui_get_mode(void)
{
    return gui_mode;
}

void gui_init(void)
{
    xTaskCreatePinnedToCore(gui_task, "guiT", 1920, NULL, 6, NULL, 1);
}
