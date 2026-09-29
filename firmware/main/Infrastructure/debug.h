#ifndef DEBUG_H
#define DEBUG_H

/**
 * Write formatted output to the debug UART.
 *
 * Output exceeding the internal buffer will be truncated.
 * This function is blocking and must not be called from interrupts
 * or code with strict timing requirements.
 */
void debug_printf(const char *format, ...)
	__attribute__((format(printf, 1, 2)));

#endif
