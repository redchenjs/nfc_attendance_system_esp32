/*
 * gui.c
 *
 *  Created on: 2018-02-13 22:57
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"
#include "esp_system.h"

#include "gfx.h"

#include "os/core.h"
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
    {ani8_160x80_gif_ptr, ani8_160x80_gif_end}  // "Updating"
#elif defined(CONFIG_SCREEN_PANEL_ST7789)
    {ani0_240x135_gif_ptr, ani0_240x135_gif_end}, // "WiFi"
    {ani1_240x135_gif_ptr, ani1_240x135_gif_end}, // "Loading"
    {ani2_240x135_gif_ptr, ani2_240x135_gif_end}, // "Success"
    {ani3_240x135_gif_ptr, ani3_240x135_gif_end}, // "NFC"
    {ani4_240x135_gif_ptr, ani4_240x135_gif_end}, // "PowerOff"
    {ani5_240x135_gif_ptr, ani5_240x135_gif_end}, // "Clock"
    {ani6_240x135_gif_ptr, ani6_240x135_gif_end}, // "Error"
    {ani7_240x135_gif_ptr, ani7_240x135_gif_end}, // "Config"
    {ani8_240x135_gif_ptr, ani8_240x135_gif_end}  // "Updating"
#endif
};
static uint8_t img_file_index = 0;

static void gui_task_handle(void *pvParameter)
{
    gdispImage gfx_image;

    gfxInit();

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
                if (xEventGroupGetBits(user_event_group) & GUI_RELOAD_BIT) {
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
    ESP_LOGE(TAG, "task failed");
    esp_restart();
}

void gui_show_image(uint8_t filename_index)
{
#ifdef CONFIG_ENABLE_GUI
    if (filename_index >= (sizeof(img_file_ptr) / 2)) {
        ESP_LOGE(TAG, "invalid filename index");
        return;
    }
    img_file_index = filename_index;
    xEventGroupSetBits(user_event_group, GUI_RELOAD_BIT);
#endif
}

void gui_init(void)
{
    xTaskCreate(gui_task_handle, "GuiT", 1024, NULL, 6, NULL);
}
