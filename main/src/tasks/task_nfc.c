#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "inc/utils/nfc-uid.h"
void task_nfc(void)
{
    for (;;) {
        nfc_get_uid();
        vTaskDelay(50 / portTICK_RATE_MS);
    }
}
