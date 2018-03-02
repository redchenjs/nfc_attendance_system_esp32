/*
 * mp3_player.c
 *
 *  Created on: 2018-02-12 20:13
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "mad.h"
#include "stream.h"
#include "frame.h"
#include "synth.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"

#include "tasks/mp3_player.h"

#define TAG "mp3_player"

static const char mp3_file_name[][32] = {
                                            "/spiffs/snd/snd0.mp3", // "叮"
                                            "/spiffs/snd/snd1.mp3", // "认证成功"
                                            "/spiffs/snd/snd2.mp3", // "认证失败"
                                            "/spiffs/snd/snd3.mp3", // "连接失败"
                                            "/spiffs/snd/snd4.mp3", // "连接超时"
                                            "/spiffs/snd/snd5.mp3"  // "网络故障"
                                        };

uint8_t mp3_player_status = MP3_PLAYER_STOPPED;

void mp3_player_play_file(uint8_t filename_index)
{
    if (mp3_player_status == MP3_PLAYER_RUNNING) {
        mp3_player_status = MP3_PLAYER_STOPPING;
        ESP_LOGW(TAG, "mp3 player is running, now stop it");
    }

    if (filename_index >= (sizeof(mp3_file_name) / 32)) {
        ESP_LOGE(TAG, "invalid filename index");
        return;
    }

    while (mp3_player_status != MP3_PLAYER_STOPPED);

    xTaskCreate(&mp3_player_task, "mp3_player_task", (1024 * 8), (void * const)mp3_file_name[filename_index], 5, NULL);
}

void mp3_player_task(void *pvParameters)
{
    char * const filename = (char * const)pvParameters;
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        ESP_LOGE(TAG, "failed to open file for reading");
        goto err4;
    }
    struct stat st;
    if (stat(filename, &st) == -1 || st.st_size == 0) {
        ESP_LOGE(TAG, "file is empty");
        goto err3;
    }
    char *mp3_file_ptr = malloc(sizeof(char) * st.st_size);
    if (mp3_file_ptr == NULL) {
        ESP_LOGE(TAG, "no enough memory");
        goto err3;
    }
    fseek(fp, 0, SEEK_SET);
    long mp3_file_len = fread(mp3_file_ptr, sizeof(char), st.st_size, fp);
    if (mp3_file_len <= 0) {
        ESP_LOGE(TAG, "read file failed");
        goto err2;
    }

    //Allocate structs needed for mp3 decoding
    struct mad_stream *stream = malloc(sizeof(struct mad_stream));
    struct mad_frame  *frame  = malloc(sizeof(struct mad_frame));
    struct mad_synth  *synth  = malloc(sizeof(struct mad_synth));

    if (stream == NULL) { ESP_LOGE(TAG, "malloc(stream) failed"); goto err1; }
    if (synth  == NULL) { ESP_LOGE(TAG, "malloc(synth) failed");  goto err1; }
    if (frame  == NULL) { ESP_LOGE(TAG, "malloc(frame) failed");  goto err1; }

    //Initialize mp3 parts
    mad_stream_init(stream);
    mad_frame_init(frame);
    mad_synth_init(synth);

    mad_stream_buffer(stream, (unsigned char *)mp3_file_ptr, mp3_file_len);

    mp3_player_status = MP3_PLAYER_RUNNING;
    while (mp3_player_status == MP3_PLAYER_RUNNING) {
        if (mad_frame_decode(frame, stream) == -1) {
            if (!MAD_RECOVERABLE(stream->error)) {
                mp3_player_status = MP3_PLAYER_STOPPING;
                break;
            }
            ESP_LOGE(TAG, "dec err 0x%04x (%s)", stream->error, mad_stream_errorstr(stream));
            continue;
        }
        mad_synth_frame(synth, frame);
    }
    // avoid noise
    i2s_zero_dma_buffer(0);

err1:
    free(synth);
    free(frame);
    free(stream);
err2:
    free(mp3_file_ptr);
err3:
    fclose(fp);
err4:
    mp3_player_status = MP3_PLAYER_STOPPED;

    vTaskDelete(NULL);
}
