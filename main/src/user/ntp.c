/*
 * ntp.c
 *
 *  Created on: 2018-02-16 17:50
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <time.h>

#include "esp_log.h"

#include "lwip/apps/sntp.h"

#include "os/core.h"
#include "user/nfc_app.h"
#include "user/gui.h"
#include "user/led.h"
#include "user/http_ota.h"

#define TAG "ntp"

static void ntp_task_handle(void *pvParameter)
{
    xEventGroupWaitBits(
        user_event_group,
        NTP_RUN_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    led_set_mode(2);
    gui_show_image(5);

    setenv("TZ", "CST-8", 1);
    tzset();

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0,"0.cn.pool.ntp.org");
    sntp_setservername(1,"1.cn.pool.ntp.org");
    sntp_setservername(2,"2.cn.pool.ntp.org");
    sntp_setservername(3,"3.cn.pool.ntp.org");
    sntp_init();

    time_t now = 0;
    struct tm timeinfo = {0};
    char strftime_buf[64];
    int retry = 1;
    const int retry_count = 15;

    while (1) {
        ESP_LOGW(TAG, "waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
        if (timeinfo.tm_year >= (2018 - 1900)) {
            break;
        }
        if (++retry > retry_count) {
            ESP_LOGE(TAG, "timeout");
            gui_show_image(4);
            vTaskDelay(2000 / portTICK_RATE_MS);
            esp_restart();
        }
    }

    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGW(TAG, "the current date/time in Shanghai is: %s", strftime_buf);

    xEventGroupSetBits(user_event_group, NTP_READY_BIT);

    while (1) {
        vTaskDelay(60000 / portTICK_RATE_MS);
        EventBits_t uxBits = xEventGroupGetBits(user_event_group);
        if (uxBits & NFC_APP_RUN_BIT) {
            time(&now);
            localtime_r(&now, &timeinfo);
            if (timeinfo.tm_hour == 0 && timeinfo.tm_min == 0) {
                nfc_app_set_mode(0);
                http_ota_update();
                nfc_app_set_mode(1);
            }
        }
    }
}

void ntp_sync_time(void)
{
    xEventGroupClearBits(os_event_group, INPUT_READY_BIT);
    EventBits_t uxBits = xEventGroupGetBits(user_event_group);
    if ((uxBits & NTP_READY_BIT) == 0) {
        xEventGroupSync(
            user_event_group,
            NTP_RUN_BIT,
            NTP_READY_BIT,
            portMAX_DELAY
        );
    }
    xEventGroupSetBits(os_event_group, INPUT_READY_BIT);
}

void ntp_init(void)
{
    xTaskCreate(ntp_task_handle, "NtpT", 2048, NULL, 5, NULL);
}
