/*
 * ntp_daemon.c
 *
 *  Created on: 2018-02-16 17:50
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "apps/sntp/sntp.h"
#include "esp_log.h"

#include "system/event.h"
#include "user/gui_daemon.h"
#include "user/led_daemon.h"
#include "user/ota_update.h"

#define TAG "ntp"

void ntp_daemon(void *pvParameter)
{
    xEventGroupWaitBits(
        daemon_event_group,
        NTP_DAEMON_READY_BIT,
        pdTRUE,
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
            ESP_LOGE(TAG, "can not wait to reboot...");
            gui_show_image(4);
            vTaskDelay(2000 / portTICK_RATE_MS);
            esp_restart();
        }
    }

    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGW(TAG, "the current date/time in Shanghai is: %s", strftime_buf);

    xEventGroupSetBits(daemon_event_group, NTP_DAEMON_FINISH_BIT);

    while (1) {
        vTaskDelay(60000 / portTICK_RATE_MS);
        EventBits_t uxBits = xEventGroupGetBits(daemon_event_group);
        if ((uxBits & NFC_DAEMON_READY_BIT) == 0) {
            continue;
        }
        time(&now);
        localtime_r(&now, &timeinfo);
        if (timeinfo.tm_hour == 0 && timeinfo.tm_min == 0) {
            ota_update();
        }
    }
}

void ntp_sync_time(void)
{
    xEventGroupClearBits(system_event_group, INPUT_READY_BIT);
    EventBits_t uxBits = xEventGroupGetBits(daemon_event_group);
    if ((uxBits & NTP_DAEMON_FINISH_BIT) == 0) {
        xEventGroupSetBits(daemon_event_group, NTP_DAEMON_READY_BIT);
        xEventGroupWaitBits(
            daemon_event_group,
            NTP_DAEMON_FINISH_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
        );
    }
    xEventGroupSetBits(system_event_group, INPUT_READY_BIT);
}
