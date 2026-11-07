#include "error.h"

#include "stm32wbxx.h"

static const char *volatile failure_message;
static const char *volatile failure_file;
static volatile uint32_t failure_line;

void error_fatal(void)
{
	if ((CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0U) {
		__BKPT(0);
	}

	NVIC_SystemReset();
}

void assert_failed(
	const char *expression,
	const char *file,
	const uint32_t line
)
{
	failure_message = expression;
	failure_file = file;
	failure_line = line;

	error_fatal();
}

void panic_failed(
	const char *reason,
	const char *file,
	const uint32_t line
)
{
	failure_message = reason;
	failure_file = file;
	failure_line = line;

	error_fatal();
}
