#include "error.h"

#include "debug.h"
#include "main.h"

void error_fatal(void)
{
	if ((CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0U) {
		__BKPT(0);
	}

	__disable_irq();
	NVIC_SystemReset();

	while (1) {
	}
}

void assert_failed(
	const char *expression,
	const char *file,
	uint32_t line
)
{
	debug_printf(
		"ASSERT: %s (%s:%lu)\r\n",
		expression,
		file,
		(unsigned long)line
	);

	error_fatal();
}

void panic_failed(
	const char *reason,
	const char *file,
	uint32_t line
)
{
	debug_printf(
		"PANIC: %s (%s:%lu)\r\n",
		reason,
		file,
		(unsigned long)line
	);

	error_fatal();
}
