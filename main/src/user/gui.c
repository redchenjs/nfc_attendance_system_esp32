/*
 * gui.c
 *
 *  Created on: 2018-02-13 22:57
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"
#include "esp_system.h"

#include "gfx.h"

#include "core/os.h"
#include "user/gui.h"

#define TAG "gui"

static const char *img_file_ptr[][2] = {
#ifdef CONFIG_SCREEN_PANEL_ST7735
    {ani0_160x80_gif_ptr, ani0_160x80_gif_end}, // "WiFi"
    {ani1_160x80_gif_ptr, ani1_160x80_gif_end}, // "Loading"
    {ani2_160x80_gif_ptr, ani2_160x80_gif_end}, // "Success"
    {ani3_160x80_gif_ptr, ani3_160x80_gif_end}, // "NFC"
    {ani4_160x80_gif_ptr, ani4_160x80_gif_end}, // "PowerOff"
    {ani5_160x80_gif_ptr, ani5_160x80_gif_end}, // "Clock"
    {ani6_160x80_gif_ptr, ani6_160x80_gif_end}, // "Error"
    {ani7_160x80_gif_ptr, ani7_160x80_gif_end}, // "Config"
    {ani8_160x80_gif_ptr, ani8_160x80_gif_end}, // "Updating"
#elif defined(CONFIG_SCREEN_PANEL_ST7789)
    {ani0_240x135_gif_ptr, ani0_240x135_gif_end}, // "WiFi"
    {ani1_240x135_gif_ptr, ani1_240x135_gif_end}, // "Loading"
    {ani2_240x135_gif_ptr, ani2_240x135_gif_end}, // "Success"
    {ani3_240x135_gif_ptr, ani3_240x135_gif_end}, // "NFC"
    {ani4_240x135_gif_ptr, ani4_240x135_gif_end}, // "PowerOff"
    {ani5_240x135_gif_ptr, ani5_240x135_gif_end}, // "Clock"
    {ani6_240x135_gif_ptr, ani6_240x135_gif_end}, // "Error"
    {ani7_240x135_gif_ptr, ani7_240x135_gif_end}, // "Config"
    {ani8_240x135_gif_ptr, ani8_240x135_gif_end}, // "Updating"
#endif
};

GDisplay *gui_gdisp = NULL;

static coord_t gui_disp_width = 0;
static coord_t gui_disp_height = 0;

static uint8_t gui_backlight = 255;

static uint8_t img_file_index = 0;

static void gui_task(void *pvParameter)
{
    portTickType xLastWakeTime;
    gdispImage gfx_image;

    gfxInit();

    gui_gdisp = gdispGetDisplay(0);
    gui_disp_width = gdispGGetWidth(gui_gdisp);
    gui_disp_height = gdispGGetHeight(gui_gdisp);

    gdispGFillArea(gui_gdisp, 0, 0, gui_disp_width, gui_disp_height, 0x000000);

    gdispGSetBacklight(gui_gdisp, gui_backlight);

    ESP_LOGI(TAG, "started.");

    while (1) {
        xEventGroupWaitBits(
            user_event_group,
            GUI_RELOAD_BIT,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY
        );

        if (!(gdispImageOpenMemory(&gfx_image, img_file_ptr[img_file_index][0]) & GDISP_IMAGE_ERR_UNRECOVERABLE)) {
            gdispImageSetBgColor(&gfx_image, Black);

            while (1) {
                xLastWakeTime = xTaskGetTickCount();

                if (xEventGroupGetBits(user_event_group) & GUI_RELOAD_BIT) {
                    break;
                }

                if (gdispImageDraw(&gfx_image, 0, 0, gfx_image.width, gfx_image.height, 0, 0) != GDISP_IMAGE_ERR_OK) {
                    goto err;
                }

                delaytime_t delay = gdispImageNext(&gfx_image);
                if (delay == TIME_INFINITE) {
                    break;
                }

                if (delay != TIME_IMMEDIATE) {
                    vTaskDelayUntil(&xLastWakeTime, delay / portTICK_RATE_MS);
                }
            }

            gdispImageClose(&gfx_image);
        } else {
            goto err;
        }
    }

err:
    ESP_LOGE(TAG, "unrecoverable error");
    esp_restart();
}

void gui_show_image(uint8_t idx)
{
#ifdef CONFIG_ENABLE_GUI
    if (idx >= (sizeof(img_file_ptr) / 2)) {
        ESP_LOGE(TAG, "invalid filename index");
        return;
    }
    img_file_index = idx;
    xEventGroupSetBits(user_event_group, GUI_RELOAD_BIT);
#endif
}

void gui_init(void)
{
    xTaskCreatePinnedToCore(gui_task, "guiT", 1536, NULL, 6, NULL, 1);
}
