#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t InputButtons;

typedef enum {
	INPUT_PLAYER_1 = 0,
	INPUT_PLAYER_2,
	INPUT_PLAYER_COUNT
} InputPlayer;

typedef struct {
	InputButtons held;
	bool connected;
} InputState;

void client_init(void);

void client_event_process(void *pckt);

InputState client_get(InputPlayer player);

#endif
