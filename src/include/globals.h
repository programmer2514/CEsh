#ifndef CESH_GLOBALS
#define CESH_GLOBALS

#include "macros.h"
#include "types.h"

extern const char *CALC_NAME;
extern const char CURSOR_INDEX[4];
extern const char DOW_NAMES[7][4];
extern const char MON_NAMES[12][4];
extern const char KEY_MAP[4][57];

extern char input[INPUT_LENGTH];
extern char path[PATH_LENGTH];
extern char user[USER_PWD_LENGTH];
extern char pwd[USER_PWD_LENGTH];

extern uint8_t textIndex;
extern uint16_t dateTime[7];

extern bool startOnNewLine;
extern bool underlineText;
extern bool italicText;
extern bool boldText;

extern bool isRetFromPrgm;
extern bool noSplash;
extern bool settingsAppvarExists;
extern bool shouldScroll;

extern ti_var_t appvarSlot;
extern char_styled_t scrBuffer[SCR_HEIGHT][SCR_WIDTH];

#endif