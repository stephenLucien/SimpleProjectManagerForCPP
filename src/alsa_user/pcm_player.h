#ifndef __PCM_PLAYER_H__
#define __PCM_PLAYER_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int simple_alsa_pcm_player_start(const char *device = "default", unsigned int samplerate = 16000, int chn = 1);

int simple_alsa_pcm_player_stop();

int simple_alsa_pcm_player_write(const uint8_t *data, int len);

#ifdef __cplusplus
}
#endif


#endif  // __PCM_PLAYER_H__
