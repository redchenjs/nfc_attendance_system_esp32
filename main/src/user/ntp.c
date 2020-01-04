/*
 * ntp.c
 *
 *  Created on: 2018-02-16 17:50
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <time.h>

#include "esp_log.h"
#include "esp_sntp.h"

#include "core/os.h"

#include "user/gui.h"
#include "user/led.h"
#include "user/nfc_app.h"
#include "user/http_app_ota.h"

#define TAG "ntp"

static time_t now = 0;
static struct tm timeinfo = {0};
static char strftime_buf[64];

static void ntp_time_sync_notification_cb(struct timeval *tv)
{
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

    ESP_LOGW(TAG, "the current date/time is: %s %s", CONFIG_NTP_TIMEZONE, strftime_buf);

    xEventGroupSetBits(user_event_group, NTP_READY_BIT);
}

static void ntp_task(void *pvParameter)
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

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, CONFIG_NTP_SERVER_URL);
    sntp_set_time_sync_notification_cb(ntp_time_sync_notification_cb);

    sntp_init();

    setenv("TZ", CONFIG_NTP_TIMEZONE, 1);
    tzset();

    ESP_LOGI(TAG, "started.");

    int retry = 1;
    const int retry_count = 15;

    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) {
        ESP_LOGW(TAG, "waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(1000 / portTICK_RATE_MS);

        if (++retry > retry_count) {
            ESP_LOGE(TAG, "time sync timeout");

            gui_show_image(4);
            vTaskDelay(2000 / portTICK_RATE_MS);

            esp_restart();
        }
    }

    while (1) {
        vTaskDelay(60000 / portTICK_RATE_MS);

        EventBits_t uxBits = xEventGroupGetBits(user_event_group);
        if (uxBits & NFC_APP_RUN_BIT) {
            time(&now);
            localtime_r(&now, &timeinfo);
            if (timeinfo.tm_hour == 0 && timeinfo.tm_min == 0) {
                nfc_app_set_mode(0);
                http_app_check_for_updates();
                nfc_app_set_mode(1);
            }
        }
    }
}

void ntp_sync_time(void)
{
    EventBits_t uxBits = xEventGroupGetBits(user_event_group);
    if ((uxBits & NTP_READY_BIT) == 0) {
        xEventGroupSync(
            user_event_group,
            NTP_RUN_BIT,
            NTP_READY_BIT,
            portMAX_DELAY
        );
    }
}

void ntp_init(void)
{
    xTaskCreatePinnedToCore(ntp_task, "NtpT", 2048, NULL, 5, NULL, 0);
}
