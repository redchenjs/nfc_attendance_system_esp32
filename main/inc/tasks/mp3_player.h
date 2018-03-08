/*
 * mp3_player.h
 *
 *  Created on: 2018-02-12 20:13
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_TASKS_MP3_PLAYER_H_
#define INC_TASKS_MP3_PLAYER_H_

#include <stdint.h>

// snd0.mp3
extern const uint8_t snd0_mp3_ptr[] asm("_binary_snd0_mp3_start");
extern const uint8_t snd0_mp3_end[] asm("_binary_snd0_mp3_end");
// snd1.mp3
extern const uint8_t snd1_mp3_ptr[] asm("_binary_snd1_mp3_start");
extern const uint8_t snd1_mp3_end[] asm("_binary_snd1_mp3_end");
// snd2.mp3
extern const uint8_t snd2_mp3_ptr[] asm("_binary_snd2_mp3_start");
extern const uint8_t snd2_mp3_end[] asm("_binary_snd2_mp3_end");
// snd3.mp3
extern const uint8_t snd3_mp3_ptr[] asm("_binary_snd3_mp3_start");
extern const uint8_t snd3_mp3_end[] asm("_binary_snd3_mp3_end");
// snd4.mp3
extern const uint8_t snd4_mp3_ptr[] asm("_binary_snd4_mp3_start");
extern const uint8_t snd4_mp3_end[] asm("_binary_snd4_mp3_end");
// snd5.mp3
extern const uint8_t snd5_mp3_ptr[] asm("_binary_snd5_mp3_start");
extern const uint8_t snd5_mp3_end[] asm("_binary_snd5_mp3_end");
// snd6.mp3
extern const uint8_t snd6_mp3_ptr[] asm("_binary_snd6_mp3_start");
extern const uint8_t snd6_mp3_end[] asm("_binary_snd6_mp3_end");

extern void mp3_player_play_file(uint8_t filename_index);
extern void mp3_player_task(void *pvParameter);

#endif /* INC_TASKS_MP3_PLAYER_H_ */
