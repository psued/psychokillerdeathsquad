#include "game_audio.h"
#include "audio.h"

static AudioVolume current_game_volume = AUDIO_VOLUME_MEDIUM;

void init_game_volume_level(void){
	audio_volume_set(current_game_volume);
}

void increase_game_volume(void){
	if (current_game_volume >= AUDIO_VOLUME_COUNT - 1) {
		return;
	}
	current_game_volume += 1;
	audio_volume_set(current_game_volume);
}

void decrease_game_volume(void){
	if (current_game_volume == AUDIO_VOLUME_MUTE) {
		return;
	}
	current_game_volume -= 1;
	audio_volume_set(current_game_volume);
}
