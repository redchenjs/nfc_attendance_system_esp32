/*
 * mp3_player.h
 *
 *  Created on: 2018-02-12 20:13
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_TASKS_MP3_PLAYER_H_
#define INC_TASKS_MP3_PLAYER_H_

enum mp3_player_status_table {
    MP3_PLAYER_RUNNING  = 0,
    MP3_PLAYER_STOPPING = 1,
    MP3_PLAYER_STOPPED  = 2
};

#include <stdint.h>

extern void mp3_player_play_file(uint8_t filename_index);
extern void mp3_player_task(void *pvParameter);

#endif /* INC_TASKS_MP3_PLAYER_H_ */
