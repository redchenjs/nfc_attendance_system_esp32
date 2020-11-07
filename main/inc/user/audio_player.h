/*
 * audio_player.h
 *
 *  Created on: 2018-02-12 20:13
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_USER_AUDIO_PLAYER_H_
#define INC_USER_AUDIO_PLAYER_H_

typedef enum {
    MP3_FILE_IDX_NOTIFY    = 0x00,
    MP3_FILE_IDX_ERROR_REQ = 0x01,
    MP3_FILE_IDX_ERROR_RSP = 0x02,
    MP3_FILE_IDX_AUTH_DONE = 0x03,
    MP3_FILE_IDX_AUTH_FAIL = 0x04,
    MP3_FILE_IDX_CONN_TOUT = 0x05,
    MP3_FILE_IDX_CONN_FAIL = 0x06,
    MP3_FILE_IDX_WIFI_CFG  = 0x07,

    MP3_FILE_IDX_MAX
} mp3_file_t;

// snd0.mp3
extern const char snd0_mp3_ptr[] asm("_binary_snd0_mp3_start");
extern const char snd0_mp3_end[] asm("_binary_snd0_mp3_end");
// snd1.mp3
extern const char snd1_mp3_ptr[] asm("_binary_snd1_mp3_start");
extern const char snd1_mp3_end[] asm("_binary_snd1_mp3_end");
// snd2.mp3
extern const char snd2_mp3_ptr[] asm("_binary_snd2_mp3_start");
extern const char snd2_mp3_end[] asm("_binary_snd2_mp3_end");
// snd3.mp3
extern const char snd3_mp3_ptr[] asm("_binary_snd3_mp3_start");
extern const char snd3_mp3_end[] asm("_binary_snd3_mp3_end");
// snd4.mp3
extern const char snd4_mp3_ptr[] asm("_binary_snd4_mp3_start");
extern const char snd4_mp3_end[] asm("_binary_snd4_mp3_end");
// snd5.mp3
extern const char snd5_mp3_ptr[] asm("_binary_snd5_mp3_start");
extern const char snd5_mp3_end[] asm("_binary_snd5_mp3_end");
// snd6.mp3
extern const char snd6_mp3_ptr[] asm("_binary_snd6_mp3_start");
extern const char snd6_mp3_end[] asm("_binary_snd6_mp3_end");
// snd7.mp3
extern const char snd7_mp3_ptr[] asm("_binary_snd7_mp3_start");
extern const char snd7_mp3_end[] asm("_binary_snd7_mp3_end");

extern void audio_player_play_file(mp3_file_t idx);

extern void audio_player_init(void);

#endif /* INC_USER_AUDIO_PLAYER_H_ */
