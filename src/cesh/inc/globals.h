#ifndef CESH_GLOBALS
#define CESH_GLOBALS

#include "macros.h"
#include "types.h"

extern const char *CALC_NAME; // OS calc name (can be edited by CERMASTR)
extern const char CURSOR_INDEX[4]; // Index of cursor states (2nd, alpha, etc.)
extern const char DOW_NAMES[7][4]; // Shortnames of the days of the week
extern const char MON_NAMES[12][4]; // Shortnames of the months of the year
extern const char KEY_MAP[4][57]; // A character map of the keyboard

extern char input[INPUT_LENGTH]; // User input buffer
extern char path[PATH_LENGTH]; // Current directory buffer
extern char user[USER_PWD_LENGTH]; // Username buffer
extern char pwd[USER_PWD_LENGTH]; // Password buffer

extern uint8_t textIndex; // Holds current cursor state
extern uint16_t dateTime[7]; // Date/time buffer

extern bool startOnNewLine; // If true, next shell loop begins by printing a newline
extern bool underlineText; // Should new text be underlined
extern bool italicText; // Should new text be italic
extern bool boldText; // Should new text be bold

extern bool isRetFromPrgm; // Has a program just finished executing
extern bool noSplash; // Disables the splash screen
extern bool settingsAppvarExists; // True if the CEshSett appvar exists
extern bool shouldScroll; // Whether the shell should scroll on the next loop

extern ti_var_t appvarSlot; // Common slot used for opening appvars
extern char_styled_t scrBuffer[SCR_HEIGHT][SCR_WIDTH]; // Styled screen buffer

#endif