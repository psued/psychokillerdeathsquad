#ifndef INPUT_H_
#define INPUT_H_

#include <stdint.h>

typedef uint8_t InputButtons;

#define INPUT_BUTTON_UP    (1U << 0)
#define INPUT_BUTTON_DOWN  (1U << 1)
#define INPUT_BUTTON_LEFT  (1U << 2)
#define INPUT_BUTTON_RIGHT (1U << 3)
#define INPUT_BUTTON_A     (1U << 4)
#define INPUT_BUTTON_B     (1U << 5)

/**
 * Initialize the input subsystem.
 */
void input_init(void);

/**
 * Sample and debounce the buttons.
 */
void input_sample(void);

/**
 * Return the stable held mask.
 */
InputButtons input_get(void);

#endif
