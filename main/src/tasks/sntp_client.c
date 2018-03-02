/*
 * sntp_client.c
 *
 *  Created on: 2018-02-16 17:50
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"

#include "apps/sntp/sntp.h"

#include "device/wifi.h"
#include "tasks/sntp_client.h"
#include "tasks/oled_display.h"

#define TAG "sntp_client"

uint8_t sntp_client_status = SNTP_TIME_NOT_SET;

void sntp_client_task(void *pvParameter)
{
    xEventGroupWaitBits(wifi0_event_group, WIFI0_CONNECTED_BIT,
                        false, true, portMAX_DELAY);
    
    setenv("TZ", "CST-8", 1);
    tzset();
    
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0,"0.cn.pool.ntp.org");
    sntp_setservername(1,"1.cn.pool.ntp.org");
    sntp_setservername(2,"2.cn.pool.ntp.org");
    sntp_setservername(3,"3.cn.pool.ntp.org");
    sntp_init();

    time_t now = 0;
    struct tm timeinfo = { 0 };
    char strftime_buf[64];
    int retry = 1;
    const int retry_count = 35;

    while (timeinfo.tm_year < (2018 - 1900)) {
        ESP_LOGW(TAG, "waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
        if (++retry > retry_count) {
            ESP_LOGW(TAG, "still waiting");
            retry = 1;
        }
    }

    sntp_client_status = SNTP_TIME_SET;

    oled_display_show_image(3);

    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGW(TAG, "the current date/time in Shanghai is: %s", strftime_buf);

    vTaskDelete(NULL);
}
