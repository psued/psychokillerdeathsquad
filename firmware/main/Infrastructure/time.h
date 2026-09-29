#ifndef TIME_H
#define TIME_H

#include <stdint.h>

/**
 * Start the microsecond system timebase.
 */
void time_init(void);

/**
 * Return the monotonic system time in milliseconds.
 *
 * The counter wraps after approximately 49.7 days.
 */
uint32_t time_ms(void);

/**
 * Return the monotonic system time in microseconds.
 *
 * The counter wraps after approximately 71.6 minutes.
 */
uint32_t time_us(void);

#endif
