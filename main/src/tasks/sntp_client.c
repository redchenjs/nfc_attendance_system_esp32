/*
 * sntp_client.c
 *
 *  Created on: 2018-02-16 17:50
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "apps/sntp/sntp.h"
#include "esp_log.h"

#include "system/event.h"
#include "tasks/gui_daemon.h"

#define TAG "sntp_client"

void sntp_client_task(void *pvParameter)
{
    xEventGroupWaitBits(system_event_group, WIFI_READY_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    
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
    const int retry_count = 10;

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
            gui_daemon_show_image(4);
            vTaskDelay(5000 / portTICK_RATE_MS);
            esp_restart();
        }
    }

    xEventGroupSetBits(task_event_group, SNTP_CLIENT_READY_BIT);

    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGW(TAG, "the current date/time in Shanghai is: %s", strftime_buf);

    vTaskDelete(NULL);
}
