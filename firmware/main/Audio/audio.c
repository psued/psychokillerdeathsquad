#include "audio.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "main.h"
#include "storage.h"

#define AUDIO_OUTPUT_SAMPLE_RATE 44100U

#define AUDIO_FRAMES_PER_HALF 512U
#define AUDIO_OUTPUT_SLOTS    2U
#define AUDIO_DMA_HALF_COUNT  2U

#define AUDIO_HALF_SAMPLES (AUDIO_FRAMES_PER_HALF * AUDIO_OUTPUT_SLOTS)
#define AUDIO_DMA_SAMPLES  (AUDIO_HALF_SAMPLES * AUDIO_DMA_HALF_COUNT)

#define AUDIO_SFX_VOICE_COUNT 4U
#define AUDIO_SFX_POOL_SIZE   (64U * 1024U)
#define AUDIO_LOAD_CHUNK_SIZE 512U

#define AUDIO_SFX_UPSAMPLE_FACTOR 2
#define AUDIO_SFX_PCM_BIAS        128
#define AUDIO_SFX_PCM_SCALE       256
#define AUDIO_GAIN_UNITY          256

#define AUDIO_MUSIC_STREAM_COUNT 2U
#define AUDIO_MUSIC_BUFFER_SIZE  (8U * 1024U)
#define AUDIO_MUSIC_FILL_SIZE    (2U * 1024U)

#define AUDIO_MUSIC_FILL_THRESHOLD (AUDIO_MUSIC_BUFFER_SIZE * 3U / 4U)
#define AUDIO_MUSIC_PREPARE_SIZE   (AUDIO_MUSIC_BUFFER_SIZE / 2U)

#define AUDIO_MUSIC_SAMPLE_BYTES 2U

#define AUDIO_CROSSFADE_MS     1000U
#define AUDIO_CROSSFADE_FRAMES (AUDIO_OUTPUT_SAMPLE_RATE * AUDIO_CROSSFADE_MS / 1000U)

extern SAI_HandleTypeDef hsai_BlockA1;

typedef enum {
	AUDIO_TYPE_SFX = 0,
	AUDIO_TYPE_MUSIC
} AudioType;

typedef struct {
	const char *path;
	AudioType type;
} AudioMapping;

typedef struct {
	uint32_t pool_offset;
	uint32_t size_bytes;
	bool loaded;
} SfxAsset;

typedef struct {
	AudioId id;
	uint32_t frame;
	uint16_t gain;
	bool active;
} SfxVoice;

typedef enum {
	MUSIC_STREAM_FREE = 0,
	MUSIC_STREAM_PREPARING,
	MUSIC_STREAM_PLAYING,
	MUSIC_STREAM_CLOSING
} MusicStreamState;

typedef struct {
	StorageFile *file;
	AudioId id;

	uint8_t buffer[AUDIO_MUSIC_BUFFER_SIZE];
	uint32_t position;
	uint32_t end;

	uint16_t gain;
	MusicStreamState state;
} MusicStream;

static const AudioMapping audio_mappings[AUDIO_COUNT] = {
	[SFX_HIT_LIGHT] = {
		.path = "AUDIO/SFX/SFX_HIT_LIGHT.PCM",
		.type = AUDIO_TYPE_SFX
	},
	[SFX_HIT_HEAVY] = {
		.path = "AUDIO/SFX/SFX_HIT_HEAVY.PCM",
		.type = AUDIO_TYPE_SFX
	},
	[SFX_BLOCK] = {
		.path = "AUDIO/SFX/SFX_BLOCK.PCM",
		.type = AUDIO_TYPE_SFX
	},
	[SFX_GRAB] = {
		.path = "AUDIO/SFX/SFX_GRAB.PCM",
		.type = AUDIO_TYPE_SFX
	},
	[SFX_SLAM] = {
		.path = "AUDIO/SFX/SFX_SLAM.PCM",
		.type = AUDIO_TYPE_SFX
	},
	[SFX_SWITCH] = {
		.path = "AUDIO/SFX/SFX_SWITCH.PCM",
		.type = AUDIO_TYPE_SFX
	},
	[SFX_SELECT] = {
		.path = "AUDIO/SFX/SFX_SELECT.PCM",
		.type = AUDIO_TYPE_SFX
	},
	[MUSIC_MENU_1] = {
		.path = "AUDIO/MUSIC/MUSIC_MENU_1.PCM",
		.type = AUDIO_TYPE_MUSIC
	},
	[MUSIC_MENU_2] = {
		.path = "AUDIO/MUSIC/MUSIC_MENU_2.PCM",
		.type = AUDIO_TYPE_MUSIC
	},
	[MUSIC_SELECT] = {
		.path = "AUDIO/MUSIC/MUSIC_SELECT.PCM",
		.type = AUDIO_TYPE_MUSIC
	},
	[MUSIC_BATTLE_BRAZIL] = {
		.path = "AUDIO/MUSIC/MUSIC_BATTLE_BRAZIL.PCM",
		.type = AUDIO_TYPE_MUSIC
	},
	[MUSIC_BATTLE_USA] = {
		.path = "AUDIO/MUSIC/MUSIC_BATTLE_USA.PCM",
		.type = AUDIO_TYPE_MUSIC
	},
	[MUSIC_BATTLE_BRITAIN] = {
		.path = "AUDIO/MUSIC/MUSIC_BATTLE_BRITAIN.PCM",
		.type = AUDIO_TYPE_MUSIC
	},
	[MUSIC_BATTLE_CHINA] = {
		.path = "AUDIO/MUSIC/MUSIC_BATTLE_CHINA.PCM",
		.type = AUDIO_TYPE_MUSIC
	},
	[MUSIC_BATTLE_JAPAN] = {
		.path = "AUDIO/MUSIC/MUSIC_BATTLE_JAPAN.PCM",
		.type = AUDIO_TYPE_MUSIC
	},
	[MUSIC_BATTLE_EGYPT] = {
		.path = "AUDIO/MUSIC/MUSIC_BATTLE_EGYPT.PCM",
		.type = AUDIO_TYPE_MUSIC
	},
	[MUSIC_BATTLE_DUNGEON] = {
		.path = "AUDIO/MUSIC/MUSIC_BATTLE_DUNGEON.PCM",
		.type = AUDIO_TYPE_MUSIC
	},
	[MUSIC_BATTLE_VOLCANO] = {
		.path = "AUDIO/MUSIC/MUSIC_BATTLE_VOLCANO.PCM",
		.type = AUDIO_TYPE_MUSIC
	},
	[MUSIC_ROUND_OVER] = {
		.path = "AUDIO/MUSIC/MUSIC_ROUND_OVER.PCM",
		.type = AUDIO_TYPE_MUSIC
	},
	[MUSIC_MATCH_OVER] = {
		.path = "AUDIO/MUSIC/MUSIC_MATCH_OVER.PCM",
		.type = AUDIO_TYPE_MUSIC
	},
	[MUSIC_CREDITS] = {
		.path = "AUDIO/MUSIC/MUSIC_CREDITS.PCM",
		.type = AUDIO_TYPE_MUSIC
	}
};

static const uint16_t audio_volume_gains[AUDIO_VOLUME_COUNT] = {
	[AUDIO_VOLUME_MUTE]    = 0U,
	[AUDIO_VOLUME_LOW]     = 16U,
	[AUDIO_VOLUME_MEDIUM]  = 32U,
	[AUDIO_VOLUME_HIGH]    = 64U,
	[AUDIO_VOLUME_FULL]    = 128U
};

static uint16_t volume_gain;

static int16_t dma_buffer[AUDIO_DMA_SAMPLES]
	__attribute__((aligned(4)));

static uint8_t sfx_pool[AUDIO_SFX_POOL_SIZE];
static uint32_t sfx_pool_used;

static SfxAsset sfx_assets[AUDIO_COUNT];
static SfxVoice sfx_voices[AUDIO_SFX_VOICE_COUNT];
static uint8_t load_buffer[AUDIO_LOAD_CHUNK_SIZE];

static MusicStream music_streams[AUDIO_MUSIC_STREAM_COUNT];

static bool music_request_pending;
static AudioId music_request_id;

static MusicStream *music_fade_from;
static MusicStream *music_fade_to;
static uint32_t music_fade_frame;

typedef enum {
	AUDIO_HALF_NONE = 0,
	AUDIO_HALF_FIRST,
	AUDIO_HALF_SECOND
} AudioHalf;

static volatile AudioHalf dma_half_ready;
static volatile bool dma_failed;

static bool available;

static Error sfx_load(const AudioId id)
{
	SfxAsset *asset = &sfx_assets[id];

	if (asset->loaded) {
		return ERROR_NONE;
	}

	StorageFile *file = NULL;
	Error error = storage_open(&file, audio_mappings[id].path);

	if (error != ERROR_NONE) {
		return error;
	}

	uint32_t size_bytes = 0U;

	while (1) {
		size_t bytes_read = 0U;

		error = storage_read(
			file,
			load_buffer,
			sizeof(load_buffer),
			&bytes_read
		);

		if (error != ERROR_NONE) {
			(void)storage_close(file);
			return error;
		}

		if (bytes_read == 0U) {
			break;
		}

		const uint32_t remaining = AUDIO_SFX_POOL_SIZE - sfx_pool_used - size_bytes;

		if (bytes_read > remaining) {
			(void)storage_close(file);
			return ERROR_UNAVAILABLE;
		}

		memcpy(
			&sfx_pool[sfx_pool_used + size_bytes],
			load_buffer,
			bytes_read
		);

		size_bytes += (uint32_t)bytes_read;
	}

	error = storage_close(file);

	if (error != ERROR_NONE) {
		return error;
	}

	if (size_bytes == 0U) {
		return ERROR_INVALID_DATA;
	}

	asset->pool_offset = sfx_pool_used;
	asset->size_bytes = size_bytes;
	asset->loaded = true;

	sfx_pool_used += size_bytes;

	return ERROR_NONE;
}

static void sfx_play(const AudioId id)
{
	if (!sfx_assets[id].loaded) {
		return;
	}

	for (size_t i = 0; i < AUDIO_SFX_VOICE_COUNT; i++) {
		SfxVoice *voice = &sfx_voices[i];

		if (voice->active) {
			continue;
		}

		voice->id = id;
		voice->frame = 0U;
		voice->gain = AUDIO_GAIN_UNITY;
		voice->active = true;

		return;
	}
}

static void sfx_stop(const AudioId id)
{
	for (size_t i = 0; i < AUDIO_SFX_VOICE_COUNT; i++) {
		SfxVoice *voice = &sfx_voices[i];

		if (voice->active && voice->id == id) {
			voice->active = false;
		}
	}
}

static int32_t sfx_mix(void)
{
	int32_t mixed = 0;

	for (size_t i = 0; i < AUDIO_SFX_VOICE_COUNT; i++) {
		SfxVoice *voice = &sfx_voices[i];

		if (!voice->active) {
			continue;
		}

		const SfxAsset *asset = &sfx_assets[voice->id];
		const uint8_t *data = &sfx_pool[asset->pool_offset];

		const uint32_t index = voice->frame / AUDIO_SFX_UPSAMPLE_FACTOR;
		const int32_t current = ((int32_t)data[index] - AUDIO_SFX_PCM_BIAS) * AUDIO_SFX_PCM_SCALE;
		int32_t sample = current;

		if (voice->frame % AUDIO_SFX_UPSAMPLE_FACTOR != 0U && index + 1U < asset->size_bytes) {
			const int32_t next = ((int32_t)data[index + 1U] - AUDIO_SFX_PCM_BIAS) * AUDIO_SFX_PCM_SCALE;
			sample = (current + next) / AUDIO_SFX_UPSAMPLE_FACTOR;
		}

		mixed += sample * voice->gain / AUDIO_GAIN_UNITY;

		voice->frame++;

		if (voice->frame >= asset->size_bytes * AUDIO_SFX_UPSAMPLE_FACTOR) {
			voice->active = false;
		}
	}

	return mixed;
}

static void music_stream_close(MusicStream *stream)
{
	ASSERT(stream != NULL);

	if (stream->file != NULL) {
		(void)storage_close(stream->file);
	}

	stream->file = NULL;
	stream->position = 0U;
	stream->end = 0U;
	stream->gain = 0U;
	stream->state = MUSIC_STREAM_FREE;
}

static void music_stream_stop(MusicStream *stream)
{
	ASSERT(stream != NULL);

	if (stream->state == MUSIC_STREAM_FREE || stream->state == MUSIC_STREAM_CLOSING) {
		return;
	}

	MusicStream *other = NULL;

	if (music_fade_from == stream) {
		other = music_fade_to;
	} else if (music_fade_to == stream) {
		other = music_fade_from;
	}

	if (other != NULL) {
		if (other->state == MUSIC_STREAM_PLAYING) {
			other->gain = AUDIO_GAIN_UNITY;
		}

		music_fade_from = NULL;
		music_fade_to = NULL;
		music_fade_frame = 0U;
	}

	stream->state = MUSIC_STREAM_CLOSING;
}

static Error music_stream_fill(MusicStream *stream)
{
	ASSERT(stream != NULL);
	ASSERT(stream->state == MUSIC_STREAM_PREPARING || stream->state == MUSIC_STREAM_PLAYING);
	ASSERT(stream->file != NULL);

	const uint32_t buffered = stream->end - stream->position;

	if (stream->position > 0U) {
		memmove(
			stream->buffer,
			&stream->buffer[stream->position],
			buffered
		);

		stream->position = 0U;
		stream->end = buffered;
	}

	size_t capacity = AUDIO_MUSIC_FILL_SIZE;
	const uint32_t free_space = AUDIO_MUSIC_BUFFER_SIZE - stream->end;

	if (capacity > free_space) {
		capacity = free_space;
	}

	if (capacity == 0U) {
		return ERROR_NONE;
	}

	size_t bytes_read = 0U;

	const Error error = storage_read(
		stream->file,
		&stream->buffer[stream->end],
		capacity,
		&bytes_read
	);

	if (error != ERROR_NONE) {
		return error;
	}

	if (bytes_read == 0U) {
		if (stream->end % AUDIO_MUSIC_SAMPLE_BYTES != 0U) {
			return ERROR_INVALID_DATA;
		}

		if (stream->state == MUSIC_STREAM_PREPARING && stream->end == 0U) {
			return ERROR_INVALID_DATA;
		}

		return storage_seek(stream->file, 0U);
	}

	stream->end += (uint32_t)bytes_read;

	return ERROR_NONE;
}

static void music_play(const AudioId id)
{
	if (music_request_pending && music_request_id == id) {
		return;
	}

	for (size_t i = 0; i < AUDIO_MUSIC_STREAM_COUNT; i++) {
		MusicStream *selected = &music_streams[i];

		if (selected->state != MUSIC_STREAM_PLAYING || selected->id != id) {
			continue;
		}

		music_request_pending = false;

		for (size_t j = 0; j < AUDIO_MUSIC_STREAM_COUNT; j++) {
			MusicStream *other = &music_streams[j];

			if (other == selected) {
				continue;
			}

			if (other->state == MUSIC_STREAM_PREPARING || other->state == MUSIC_STREAM_PLAYING) {
				music_stream_stop(other);
			}
		}

		selected->gain = AUDIO_GAIN_UNITY;
		return;
	}

	for (size_t i = 0; i < AUDIO_MUSIC_STREAM_COUNT; i++) {
		MusicStream *stream = &music_streams[i];

		if (stream->state == MUSIC_STREAM_PREPARING) {
			music_stream_stop(stream);
		}
	}

	music_request_id = id;
	music_request_pending = true;
}

static void music_stop(const AudioId id)
{
	if (music_request_pending && music_request_id == id) {
		music_request_pending = false;
	}

	for (size_t i = 0; i < AUDIO_MUSIC_STREAM_COUNT; i++) {
		MusicStream *stream = &music_streams[i];

		if (stream->state != MUSIC_STREAM_FREE && stream->id == id) {
			music_stream_stop(stream);
		}
	}
}

static void music_request_process(void)
{
	if (!music_request_pending) {
		return;
	}

	MusicStream *preparing = NULL;
	MusicStream *free_stream = NULL;
	MusicStream *current = NULL;

	for (size_t i = 0; i < AUDIO_MUSIC_STREAM_COUNT; i++) {
		MusicStream *stream = &music_streams[i];

		if (stream->state == MUSIC_STREAM_PREPARING && stream->id == music_request_id) {
			preparing = stream;
		}

		if (stream->state == MUSIC_STREAM_FREE && free_stream == NULL) {
			free_stream = stream;
		}

		if (stream->state == MUSIC_STREAM_PLAYING && current == NULL) {
			current = stream;
		}
	}

	if (preparing == NULL) {
		if (free_stream == NULL) {
			return;
		}

		StorageFile *file = NULL;

		const Error error = storage_open(&file, audio_mappings[music_request_id].path);

		if (error != ERROR_NONE) {
			music_request_pending = false;
			return;
		}

		free_stream->file = file;
		free_stream->id = music_request_id;
		free_stream->position = 0U;
		free_stream->end = 0U;
		free_stream->gain = 0U;
		free_stream->state = MUSIC_STREAM_PREPARING;

		return;
	}

	const uint32_t buffered = preparing->end - preparing->position;

	if (buffered < AUDIO_MUSIC_PREPARE_SIZE) {
		const Error error = music_stream_fill(preparing);

		if (error != ERROR_NONE) {
			music_stream_close(preparing);
			music_request_pending = false;
		}

		return;
	}

	preparing->state = MUSIC_STREAM_PLAYING;
	music_request_pending = false;

	if (current == NULL) {
		preparing->gain = AUDIO_GAIN_UNITY;
		return;
	}

	preparing->gain = 0U;
	current->gain = AUDIO_GAIN_UNITY;

	music_fade_from = current;
	music_fade_to = preparing;
	music_fade_frame = 0U;
}

static void music_fade_process(void)
{
	if (music_fade_to == NULL) {
		ASSERT(music_fade_from == NULL);
		return;
	}

	ASSERT(music_fade_from != NULL);

	music_fade_frame += AUDIO_FRAMES_PER_HALF;

	if (music_fade_frame >= AUDIO_CROSSFADE_FRAMES) {
		music_fade_from->state = MUSIC_STREAM_CLOSING;
		music_fade_to->gain = AUDIO_GAIN_UNITY;

		music_fade_from = NULL;
		music_fade_to = NULL;
		music_fade_frame = 0U;

		return;
	}

	const uint16_t gain = (uint16_t)(music_fade_frame * AUDIO_GAIN_UNITY / AUDIO_CROSSFADE_FRAMES);

	music_fade_to->gain = gain;
	music_fade_from->gain = AUDIO_GAIN_UNITY - gain;
}

static int32_t music_mix(void)
{
	int32_t mixed = 0;

	for (size_t i = 0; i < AUDIO_MUSIC_STREAM_COUNT; i++) {
		MusicStream *stream = &music_streams[i];

		if (stream->state != MUSIC_STREAM_PLAYING) {
			continue;
		}

		if (stream->position + AUDIO_MUSIC_SAMPLE_BYTES > stream->end) {
			continue;
		}

		const uint16_t low = stream->buffer[stream->position];
		const uint16_t high = stream->buffer[stream->position + 1U];
		const int16_t sample = (int16_t)(low | (high << 8));

		mixed += (int32_t)sample * stream->gain / AUDIO_GAIN_UNITY;

		stream->position += AUDIO_MUSIC_SAMPLE_BYTES;
	}

	return mixed;
}

static void music_process(void)
{
	for (size_t i = 0; i < AUDIO_MUSIC_STREAM_COUNT; i++) {
		MusicStream *stream = &music_streams[i];

		if (stream->state != MUSIC_STREAM_PLAYING) {
			continue;
		}

		const uint32_t buffered = stream->end - stream->position;

		if (buffered >= AUDIO_MUSIC_FILL_THRESHOLD) {
			continue;
		}

		const Error error = music_stream_fill(stream);

		if (error != ERROR_NONE) {
			music_stream_stop(stream);
		}

		return;
	}

	for (size_t i = 0; i < AUDIO_MUSIC_STREAM_COUNT; i++) {
		MusicStream *stream = &music_streams[i];

		if (stream->state == MUSIC_STREAM_CLOSING) {
			music_stream_close(stream);
			return;
		}
	}

	music_request_process();
}

static Error audio_output_start(void)
{
	const HAL_StatusTypeDef status = HAL_SAI_Transmit_DMA(
		&hsai_BlockA1,
		(uint8_t *)dma_buffer,
		AUDIO_DMA_SAMPLES
	);

	if (status == HAL_BUSY) {
		return ERROR_BUSY;
	}

	if (status != HAL_OK) {
		return ERROR_IO;
	}

	available = true;

	HAL_GPIO_WritePin(
		AUDIO_EN_GPIO_Port,
		AUDIO_EN_Pin,
		GPIO_PIN_SET
	);

	return ERROR_NONE;
}

static Error audio_output_restart(void)
{
	available = false;

	HAL_GPIO_WritePin(
		AUDIO_EN_GPIO_Port,
		AUDIO_EN_Pin,
		GPIO_PIN_RESET
	);

	if (HAL_SAI_DMAStop(&hsai_BlockA1) != HAL_OK) {
		return ERROR_IO;
	}

	memset(dma_buffer, 0, sizeof(dma_buffer));

	const uint32_t primask = __get_PRIMASK();
	__disable_irq();

	dma_failed = false;
	dma_half_ready = AUDIO_HALF_NONE;

	__set_PRIMASK(primask);

	return audio_output_start();
}

static void audio_output_disable(void)
{
	available = false;

	HAL_GPIO_WritePin(
		AUDIO_EN_GPIO_Port,
		AUDIO_EN_Pin,
		GPIO_PIN_RESET
	);

	(void)HAL_SAI_DMAStop(&hsai_BlockA1);

	for (size_t i = 0; i < AUDIO_MUSIC_STREAM_COUNT; i++) {
		music_stream_close(&music_streams[i]);
	}

	music_request_pending = false;

	music_fade_from = NULL;
	music_fade_to = NULL;
	music_fade_frame = 0U;

	const uint32_t primask = __get_PRIMASK();
	__disable_irq();

	dma_failed = false;
	dma_half_ready = AUDIO_HALF_NONE;

	__set_PRIMASK(primask);
}

static void audio_half_fill(const AudioHalf half)
{
	ASSERT(half == AUDIO_HALF_FIRST || half == AUDIO_HALF_SECOND);

	size_t offset = 0U;

	if (half == AUDIO_HALF_SECOND) {
		offset = AUDIO_HALF_SAMPLES;
	}

	for (size_t frame = 0; frame < AUDIO_FRAMES_PER_HALF; frame++) {
		int32_t mixed = sfx_mix() + music_mix();

		mixed = mixed * volume_gain / AUDIO_GAIN_UNITY;

		if (mixed > INT16_MAX) {
			mixed = INT16_MAX;
		} else if (mixed < INT16_MIN) {
			mixed = INT16_MIN;
		}

		const int16_t output = (int16_t)mixed;
		const size_t index = offset + frame * AUDIO_OUTPUT_SLOTS;

		dma_buffer[index] = output;
		dma_buffer[index + 1U] = output;
	}

	music_fade_process();
}

Error audio_init(void)
{
	ASSERT(!available);

	available = false;
	dma_failed = false;
	dma_half_ready = AUDIO_HALF_NONE;

	memset(dma_buffer, 0, sizeof(dma_buffer));
	memset(sfx_assets, 0, sizeof(sfx_assets));
	memset(sfx_voices, 0, sizeof(sfx_voices));
	memset(music_streams, 0, sizeof(music_streams));

	sfx_pool_used = 0U;

	volume_gain = audio_volume_gains[AUDIO_VOLUME_MEDIUM];

	music_request_pending = false;

	music_fade_from = NULL;
	music_fade_to = NULL;
	music_fade_frame = 0U;

	HAL_GPIO_WritePin(
		AUDIO_EN_GPIO_Port,
		AUDIO_EN_Pin,
		GPIO_PIN_RESET
	);

	return audio_output_start();
}

void audio_process(void)
{
	if (dma_failed) {
		const Error error = audio_output_restart();

		if (error != ERROR_NONE) {
			audio_output_disable();
		}

		return;
	}

	if (!available) {
		return;
	}

	const uint32_t primask = __get_PRIMASK();
	__disable_irq();

	const AudioHalf half = dma_half_ready;
	dma_half_ready = AUDIO_HALF_NONE;

	__set_PRIMASK(primask);

	if (half != AUDIO_HALF_NONE) {
		audio_half_fill(half);
	}

	music_process();
}

Error audio_load(const AudioId id)
{
	ASSERT(id >= SFX_HIT_LIGHT && id < AUDIO_COUNT);
	ASSERT(audio_mappings[id].type == AUDIO_TYPE_SFX);

	return sfx_load(id);
}

void audio_free(void)
{
	memset(sfx_voices, 0, sizeof(sfx_voices));
	memset(sfx_assets, 0, sizeof(sfx_assets));

	sfx_pool_used = 0U;
}

void audio_volume_set(const AudioVolume volume)
{
	ASSERT(volume >= AUDIO_VOLUME_MUTE);
	ASSERT(volume < AUDIO_VOLUME_COUNT);

	volume_gain = audio_volume_gains[volume];
}

void audio_play(const AudioId id)
{
	ASSERT(id >= SFX_HIT_LIGHT && id < AUDIO_COUNT);

	if (!available) {
		return;
	}

	if (audio_mappings[id].type == AUDIO_TYPE_SFX) {
		sfx_play(id);
	} else {
		music_play(id);
	}
}

void audio_stop(const AudioId id)
{
	ASSERT(id >= SFX_HIT_LIGHT && id < AUDIO_COUNT);

	if (audio_mappings[id].type == AUDIO_TYPE_SFX) {
		sfx_stop(id);
	} else {
		music_stop(id);
	}
}

void audio_dma_half(void)
{
	dma_half_ready = AUDIO_HALF_FIRST;
}

void audio_dma_complete(void)
{
	dma_half_ready = AUDIO_HALF_SECOND;
}

void audio_dma_error(void)
{
	dma_failed = true;
}
