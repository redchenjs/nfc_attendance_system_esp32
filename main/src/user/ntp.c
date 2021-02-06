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

#include "user/led.h"
#include "user/gui.h"
#include "user/nfc_app.h"
#include "user/http_app_ota.h"

#define TAG "ntp"

static void ntp_time_sync_notification_cb(struct timeval *tv)
{
    time_t now = 0;
    struct tm timeinfo = {0};
    char strftime_buf[64] = {0};

    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

    ESP_LOGW(TAG, "current timezone: %s", CONFIG_NTP_TIMEZONE);
    ESP_LOGW(TAG, "current date/time: %s", strftime_buf);

    xEventGroupSetBits(user_event_group, NTP_SYNC_SET_BIT);
}

static void ntp_task(void *pvParameter)
{
    xEventGroupWaitBits(
        user_event_group,
        NTP_SYNC_RUN_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

#ifdef CONFIG_ENABLE_LED
    led_set_mode(LED_MODE_IDX_BLINK_M1);
#endif
#ifdef CONFIG_ENABLE_GUI
    gui_set_mode(GUI_MODE_IDX_GIF_CLK);
#endif

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, CONFIG_NTP_SERVER_URL);
    sntp_set_time_sync_notification_cb(ntp_time_sync_notification_cb);

    sntp_init();

    setenv("TZ", CONFIG_NTP_TIMEZONE, 1);
    tzset();

    ESP_LOGI(TAG, "started.");

    int retry = 0;
    const int max_retry = 15;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) {
        if (++retry > max_retry) {
            ESP_LOGE(TAG, "time sync timeout");

#ifdef CONFIG_ENABLE_GUI
            gui_set_mode(GUI_MODE_IDX_GIF_PWR);
            vTaskDelay(2000 / portTICK_RATE_MS);
#endif
            os_pwr_reset_wait(OS_PWR_DUMMY_BIT);
            break;
        }

        ESP_LOGW(TAG, "waiting for system time to be set.... (%d/%d)", retry, max_retry);

        vTaskDelay(1000 / portTICK_RATE_MS);
    }

#ifdef CONFIG_ENABLE_OTA
    time_t now = 0;
    struct tm timeinfo = {0};
    while (1) {
        vTaskDelay(60000 / portTICK_RATE_MS);

        if (nfc_app_get_mode() == NFC_APP_MODE_IDX_ON) {
            time(&now);
            localtime_r(&now, &timeinfo);
            if (timeinfo.tm_hour == 0 && timeinfo.tm_min == 0) {
                nfc_app_set_mode(NFC_APP_MODE_IDX_OFF);
                http_app_check_for_updates();
                nfc_app_set_mode(NFC_APP_MODE_IDX_ON);
            }
        }
    }
#else
    vTaskDelete(NULL);
#endif
}

void ntp_sync_time(void)
{
    if (!(xEventGroupGetBits(user_event_group) & NTP_SYNC_SET_BIT)) {
        xEventGroupSync(
            user_event_group,
            NTP_SYNC_RUN_BIT,
            NTP_SYNC_SET_BIT,
            portMAX_DELAY
        );
    }
}

void ntp_init(void)
{
    xTaskCreatePinnedToCore(ntp_task, "ntpT", 2048, NULL, 5, NULL, 0);
}
