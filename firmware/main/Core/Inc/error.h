#ifndef ERROR_H
#define ERROR_H

#include <stdint.h>

/**
 * Generic errors returned by recoverable operations.
 */
typedef enum {
	ERROR_NONE = 0,
	ERROR_TIMEOUT,
	ERROR_IO,
	ERROR_INVALID_DATA,
	ERROR_NOT_FOUND,
	ERROR_UNAVAILABLE,
	ERROR_BUSY
} Error;

/**
 * Terminates execution after an unrecoverable low-level failure.
 *
 * If a debugger is attached, execution stops at a breakpoint.
 * Otherwise, or after continuing from the breakpoint, the MCU resets.
 */
void error_fatal(void)
	__attribute__((noreturn));

/**
 * Handles a failed assertion.
 */
void assert_failed(
	const char *expression,
	const char *file,
	uint32_t line
) __attribute__((noreturn));

/**
 * Handles an unrecoverable runtime failure.
 */
void panic_failed(
	const char *reason,
	const char *file,
	uint32_t line
) __attribute__((noreturn));

/**
 * Verifies a programming invariant.
 */
#define ASSERT(condition)                                            \
	do {                                                             \
		if (!(condition)) {                                          \
			assert_failed(#condition, __FILE__, (uint32_t)__LINE__); \
		}                                                            \
	} while (0)

/**
 * Reports an unrecoverable runtime failure.
 */
#define PANIC(reason) \
	panic_failed((reason), __FILE__, (uint32_t)__LINE__)

#endif
