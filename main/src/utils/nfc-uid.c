#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "buses/emdev.h"

#include "nfc/nfc.h"

#include "inc/system/fonts.h"
#include "inc/device/led.h"
#include "inc/driver/ssd1306.h"

#define MAX_TARGET_COUNT 16

extern nfc_emdev emdev;   
   
//static nfc_device *pnd;

static uint8_t lock_status = 0;

nfc_target ant[MAX_TARGET_COUNT];

int res = 0;  
bool verbose = true; 

static char str_uid[16] = "50  76  12  1a";
char str[16] = {0};
static uint8_t attach_count = 0;
static uint8_t detach_count = 0;
  
int nfc_get_uid(void)
{
    nfc_device *pnd;
    nfc_modulation nm;
    
    int n;
    int i, cnt;
    char *s;

    pnd = nfc_open(&emdev);

    if (pnd == NULL) {
        return -1;
    }
    if (nfc_initiator_init(pnd) < 0) {
        nfc_perror(pnd, "nfc_initiator_init");
        return -1;
    }
     
    nm.nmt = NMT_ISO14443A;
    nm.nbr = NBR_106;

    if ((res = nfc_initiator_list_passive_targets(pnd, nm, ant, MAX_TARGET_COUNT)) >= 0) {
        ssd1306_clear_screen(0x00);
        
        if (res != 0) {
            for (n = 0; n < res; n++) {
                if (str_nfc_target(&s, &ant[n], verbose) < 0) {
                    printf("ram size not enough\n");
                    break;
                }
                i = 0;
                cnt = 0;

                while (s[i] != '\0') {
                    if (s[i++] == '\n') {
                        cnt++;
                        if (cnt == 4) {
                            strncpy(str, s+i+21, 14);
                            str[14] = '\0';
                            break;
                        }
                    }
                }

                if (strcmp(str_uid, str) == 0) {
                    attach_count++;
                    if (attach_count >= 2) {
                        attach_count = 2;      
                    }
                    if (attach_count >= 2 && detach_count >= 2) {
                        attach_count = 0;  
                        detach_count = 0;
                        if (lock_status == 0)
                            lock_status = 1;
                        else
                            lock_status = 0;
                    }
                }
                else {
                    attach_count = 0;
                    detach_count++;
                    if (detach_count >= 2) {
                        detach_count = 2;
                    }
                }
                
                printf("UID: %s\n", str);
                ssd1306_display_string(6, 2, "UID:", FONT_1206, NORMAL); 
                ssd1306_display_string(36, 2, str, FONT_1206, NORMAL);

                nfc_free(s);
            }
        }
        else {
            attach_count = 0;
            detach_count++;
            if (detach_count >= 2) {
                detach_count = 2;
            }
            printf("None\n");
            ssd1306_display_string(51, 2, "None", FONT_1206, NORMAL);        
        }

        printf("attach_count: %d detach_count: %d\n", attach_count, detach_count);
        
        if (lock_status == 1) {
            led_on();
            printf("Unlocked\n");
            ssd1306_display_string(34, 30, "Unlocked", FONT_1608, NORMAL);
        }
        else {
            led_off();
            printf("Locked\n");
            ssd1306_display_string(40, 30, "Locked", FONT_1608, NORMAL);
        }
        printf("\n");
        ssd1306_refresh_gram();
    }

    nfc_close(pnd);

    return 0;
}
