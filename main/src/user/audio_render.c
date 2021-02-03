/*
 * audio_render.c
 *
 *  Created on: 2018-04-05 16:41
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"

#include "driver/i2s.h"

#include "chip/i2s.h"

/* render callback for the libmad synth */
void render_sample_block(short *sample_ch0, short *sample_ch1, unsigned int sample_rate, unsigned int nch, unsigned int ns)
{
    if (nch == 1) {
        sample_ch1 = sample_ch0;
    }

    i2s_output_set_sample_rate(sample_rate);

    size_t bytes_written = 0;
    for (int i = 0; i < ns; i++) {
        i2s_write(CONFIG_AUDIO_OUTPUT_I2S_NUM, sample_ch0++, sizeof(short), &bytes_written, portMAX_DELAY);
        i2s_write(CONFIG_AUDIO_OUTPUT_I2S_NUM, sample_ch1++, sizeof(short), &bytes_written, portMAX_DELAY);
    }
}
