#include <stdint.h>
#include "game_random.h"

static uint32_t state = 0x12345678U;

void random_seed(uint32_t seed) {
    state = (seed != 0U) ? seed : 0x12345678U;
}

uint8_t random_bit(void){
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return (uint8_t)(state >> 31);
}