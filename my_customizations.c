// Board customization.
//
// Enable any of these function if you need more or different setup code.
//
#include <stdbool.h>

#if 0
void board_setup_early(void)
{
	// called immediately once we decide to go the DFU route
	volatile int a = 34;
}
#endif

#if 0
void board_setup_late(void)
{
	// called after NVM and some other chip modules are init'ed
}
#endif

#if 0
void board_reset_cleanup(void)
{
	// called after DFU has been loaded and we're going to run
	// the new firmware; might undo various setup that was in place.
}
#endif

#if 0
bool button_pressed(void)
{
	// Return T to do DFU rather than run normal firmware.
	return false;
}
#endif
