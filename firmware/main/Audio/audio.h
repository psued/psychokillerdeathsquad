#ifndef AUDIO_H
#define AUDIO_H

#include "error.h"

typedef enum {
	SFX_HIT = 0,
	SFX_BLOCK,
	SFX_MENU_SELECT,
	MUSIC_MENU,
	MUSIC_BATTLE,
	AUDIO_COUNT
} AudioId;

typedef enum {
	AUDIO_VOLUME_MUTE = 0,
	AUDIO_VOLUME_LOW,
	AUDIO_VOLUME_MEDIUM,
	AUDIO_VOLUME_HIGH,
	AUDIO_VOLUME_FULL,
	AUDIO_VOLUME_COUNT
} AudioVolume;

/**
 * Initialize audio output.
 */
Error audio_init(void);

/**
 * Process audio output.
 */
void audio_process(void);

/**
 * Load an SFX asset into memory.
 */
Error audio_load(AudioId id);

/**
 * Stop and free all loaded SFX assets.
 */
void audio_free(void);

/**
 * Set the output volume.
 */
void audio_volume_set(AudioVolume volume);

/**
 * Play an audio asset.
 */
void audio_play(AudioId id);

/**
 * Stop an audio asset.
 */
void audio_stop(AudioId id);

/**
 * Record SAI DMA events.
 */
void audio_dma_half(void);
void audio_dma_complete(void);
void audio_dma_error(void);

#endif
