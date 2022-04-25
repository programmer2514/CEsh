#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fileioc.h>

#include "globals.h"

const char *CALC_NAME = (char *)0x3B000E;
const char CURSOR_INDEX[4] = {129, 130, 128, 95};
const char DOW_NAMES[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char MON_NAMES[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char KEY_MAP[4][57] = {{0,0,0,0,0,0,0,0,0,0,'+' ,'-','*','/','^',0,0,'_','3','6','9',')',0  ,0,0,'.','2','5','8','(',0  ,0  ,0,'0','1','4','7',',',0  ,0  ,0,0,0  ,0  ,0  ,0  ,'\\',0  ,0,0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0,0,0,0   ,']','[','&','|',0,0,0  ,0  ,0  ,0  ,'}',0  ,0,0,'!',0  ,0  ,0  ,'{',0  ,0  ,0,0  ,0  ,0  ,0  ,'=',0  ,0  ,0,0,0  ,0  ,0  ,0  ,0   ,0  ,0,0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0,0,0,'\'','W','R','M','H',0,0,'?','@','V','Q','L','G',0,0,':','Z','U','P','K','F','C',0,' ','Y','T','O','J','E','B',0,0,'X','S','N','I','D' ,'A',0,0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0,0,0,'"' ,'w','r','m','h',0,0,0  ,0  ,'v','q','l','g',0,0,';','z','u','p','k','f','c',0,' ','y','t','o','j','e','b',0,0,'x','s','n','i','d' ,'a',0,0,0,0,0,0,0,0,0}};

char input[INPUT_LENGTH];
char path[PATH_LENGTH];
char user[USER_PWD_LENGTH];
char pwd[USER_PWD_LENGTH];

uint8_t textIndex = 0;
uint16_t dateTime[7];

bool startOnNewLine = true;
bool underlineText = false;
bool italicText = false;
bool boldText = false;

bool isRetFromPrgm = false;
bool noSplash = false;
bool settingsAppvarExists = true;
bool shouldScroll;

ti_var_t appvarSlot;
char_styled_t scrBuffer[SCR_HEIGHT][SCR_WIDTH];