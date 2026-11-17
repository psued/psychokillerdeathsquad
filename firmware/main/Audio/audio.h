#ifndef AUDIO_H
#define AUDIO_H

#include "error.h"

typedef enum {
	SFX_HIT = 0,
	SFX_BLOCK,
	SFX_GRAB,
	SFX_SLAM,
	SFX_SWITCH,
	SFX_SELECT,
	MUSIC_MENU,
	MUSIC_SELECT,
	MUSIC_BATTLE_BRAZIL,
	MUSIC_BATTLE_USA,
	MUSIC_BATTLE_BRITAIN,
	MUSIC_BATTLE_CHINA,
	MUSIC_BATTLE_JAPAN,
	MUSIC_BATTLE_EGYPT,
	MUSIC_BATTLE_DUNGEON,
	MUSIC_BATTLE_VOLCANO,
	MUSIC_ROUND_OVER,
	MUSIC_MATCH_OVER,
	MUSIC_CREDITS,
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
