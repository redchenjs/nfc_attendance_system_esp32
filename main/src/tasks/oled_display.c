/*
 * oled_display.c
 *
 *  Created on: 2018-02-13 22:57
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <stdio.h>
#include <sys/stat.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "tasks/oled_display.h"

#include "gfx.h"

#define TAG "oled_display"

static const char img_file_name[][32] = {
                                            "/spiffs/ani/ani0.gif", // "WiFi"
                                            "/spiffs/ani/ani1.gif", // "Loading"
                                            "/spiffs/ani/ani2.gif", // "Success"
                                            "/spiffs/ani/ani3.gif"  // "NFC"
                                        };

uint8_t oled_display_status = OLED_DISPLAY_RELOAD;
uint8_t oled_display_image  = 0;

void oled_display_show_image(uint8_t filename_index)
{
    if (filename_index >= (sizeof(img_file_name) / 32)) {
        ESP_LOGE(TAG, "invalid filename index");
        return;
    }
    oled_display_image = filename_index;
    if (oled_display_status == OLED_DISPLAY_RUNNING) {
        oled_display_status = OLED_DISPLAY_RELOAD;
        ESP_LOGD(TAG, "now reload image");
    }
}

void oled_display_task(void *pvParameter)
{   
	gfxInit();

    while (1) {
        gdispImage gfx_image;
        if (oled_display_status == OLED_DISPLAY_RELOAD) {
            oled_display_status = OLED_DISPLAY_RUNNING;
        } else {
            const char *filename = img_file_name[oled_display_image];
            FILE *fp = fopen(filename, "rb");
            if (fp == NULL) {
                ESP_LOGE(TAG, "failed to open file for reading");
                goto err3;
            }
            struct stat st;
            if (stat(filename, &st) == -1 || st.st_size == 0) {
                ESP_LOGE(TAG, "file is empty");
                goto err2;
            }
            char *img_file_ptr = malloc(sizeof(char) * st.st_size);
            if (img_file_ptr == NULL) {
                ESP_LOGE(TAG, "no enough memory");
                goto err2;
            }
            fseek(fp, 0, SEEK_SET);
            long img_file_len = fread(img_file_ptr, sizeof(char), st.st_size, fp);
            if (img_file_len <= 0) {
                ESP_LOGE(TAG, "read file failed");
                goto err1;
            }
            if (!(gdispImageOpenMemory(&gfx_image, img_file_ptr) & GDISP_IMAGE_ERR_UNRECOVERABLE)) {
                gdispImageSetBgColor(&gfx_image, White);
                while (oled_display_status == OLED_DISPLAY_RUNNING) {
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
            }
        err1:
            free(img_file_ptr);
        err2:
            fclose(fp);
        err3:
            continue;
        }
    }

    ESP_LOGE(TAG, "task failed");

    oled_display_status = OLED_DISPLAY_STOPPED;

    vTaskDelete(NULL);
}
