#include <string.h>
#include <ctype.h>

#include <sys/power.h>
#include <ti/vars.h>

#include <graphx.h>
#include <fontlibc.h>
#include <fileioc.h>
#include <keypadc.h>

#include <intce.h>

#include "globals.h"
#include "macros.h"
#include "types.h"
#include "input.h"
#include "shell.h"

#include "routines.h"

uint16_t str_to_num(const char *string, const uint8_t length, const uint8_t base) {

    uint16_t result = 0,
             j = 1;
    uint8_t i;

    // Loop through string, convert each ASCII character to it's decimal form, and multiply it times the current place value
    for (i = length; i--; j *= base) {
        result += j * (string[i]
            - (inRange(string[i],'0','9') ? '0' : 0)
            - (inRange(string[i],'A','Z') ? 'A' - 10 : 0)
            - (inRange(string[i],'a','z') ? 'a' - 10 : 0)
        );
    }

    return result;
}

int run_prgm(uint8_t numargs, uint8_t *arglocs) {

    int ret;
    uint16_t cursorPos[2] = {SCR_OFFSET_X, SCR_OFFSET_Y};
    uint8_t textColors[2] = {WHITE, BLACK};
    bool sdUnderlineText = false,
         sdItalicText = false,
         sdBoldText = false;

    char *prgm = &input[2];

    // Convert program name to uppercase
    char *s = prgm;
    while (*s) {
        *s = toupper(*s);
        s++;
    }

    // Prevent shell running itself
    if (!strcmp(prgm, "CESH"))
        return -3;

    // Make sure program actually exists
    appvarSlot = ti_OpenVar(prgm, "r", OS_TYPE_PRGM);
    if (appvarSlot == 0)
        return -1;
    ti_Close(appvarSlot);

    // Set save state variables
    isRetFromPrgm = true;
    cursorPos[0] = fontlib_GetCursorX();
    cursorPos[1] = fontlib_GetCursorY();
    textColors[0] = fontlib_GetForegroundColor();
    textColors[1] = fontlib_GetBackgroundColor();
    sdUnderlineText = underlineText;
    sdItalicText = italicText;
    sdBoldText = boldText;

    // Save shell state
    appvarSlot = ti_Open("CEshSett", "r+");

    ti_Seek(sizeof(char) * (2 * USER_PWD_LENGTH), SEEK_SET, appvarSlot);
    ti_Write(&isRetFromPrgm, sizeof(bool), 1, appvarSlot);
    ti_Write(&cursorPos, sizeof(uint16_t), 2, appvarSlot);
    ti_Write(&textColors, sizeof(uint8_t), 2, appvarSlot);
    ti_Seek(sizeof(uint16_t) * 7, SEEK_SET, appvarSlot);
    ti_Write(&sdUnderlineText, sizeof(bool), 1, appvarSlot);
    ti_Write(&sdItalicText, sizeof(bool), 1, appvarSlot);
    ti_Write(&sdBoldText, sizeof(bool), 1, appvarSlot);

    ti_SetArchiveStatus(true, appvarSlot);
    ti_Close(appvarSlot);

    // Save passed args
    appvarSlot = ti_Open("CEshArgs", "w+");

    ti_Write(&input, sizeof(char), INPUT_LENGTH, appvarSlot);
    ti_Write(&numargs, sizeof(uint8_t), 1, appvarSlot);
    ti_Write(arglocs, sizeof(uint8_t), numargs, appvarSlot);

    ti_SetArchiveStatus(true, appvarSlot);
    ti_Close(appvarSlot);

    // Save screen state
    appvarSlot = ti_Open("CEshSBuf", "w+");
    ti_Write(&scrBuffer, sizeof(char_styled_t), BUFFER_SIZE, appvarSlot);
    ti_Close(appvarSlot);

    gfx_End();

    ret = os_RunPrgm(prgm, NULL, 0, (os_runprgm_callback_t)sh_main);
    sh_init();

    return ret;
}

void power_down(bool restart, bool save) {

    uint24_t enableConfigBak;
    uint16_t cursorPos[2] = {SCR_OFFSET_X, SCR_OFFSET_Y};
    uint8_t textColors[2] = {WHITE, BLACK};
    bool sdUnderlineText = false,
         sdItalicText = false,
         sdBoldText = false;

    if (save) {
        // Set save state variables
        isRetFromPrgm = true;
        cursorPos[0] = fontlib_GetCursorX();
        cursorPos[1] = fontlib_GetCursorY();
        textColors[0] = fontlib_GetForegroundColor();
        textColors[1] = fontlib_GetBackgroundColor();
        sdUnderlineText = underlineText;
        sdItalicText = italicText;
        sdBoldText = boldText;

        // Save shell state
        appvarSlot = ti_Open("CEshSett", "r+");

        ti_Seek(sizeof(char) * (2 * USER_PWD_LENGTH), SEEK_SET, appvarSlot);
        ti_Write(&isRetFromPrgm, sizeof(bool), 1, appvarSlot);
        ti_Write(&cursorPos, sizeof(uint16_t), 2, appvarSlot);
        ti_Write(&textColors, sizeof(uint8_t), 2, appvarSlot);
        ti_Seek(sizeof(uint16_t) * 7, SEEK_SET, appvarSlot);
        ti_Write(&sdUnderlineText, sizeof(bool), 1, appvarSlot);
        ti_Write(&sdItalicText, sizeof(bool), 1, appvarSlot);
        ti_Write(&sdBoldText, sizeof(bool), 1, appvarSlot);

        ti_SetArchiveStatus(true, appvarSlot);
        ti_Close(appvarSlot);

        // Save screen state
        appvarSlot = ti_Open("CEshSBuf", "w+");
        ti_Write(&scrBuffer, sizeof(char_styled_t), BUFFER_SIZE, appvarSlot);
        ti_Close(appvarSlot);
    }

    gfx_End();
    boot_TurnOff();

    if (!restart) {
        // Save and set up interrupts
        int_Enable();
        kb_ClearOnLatch();
        enableConfigBak = int_EnableConfig;
        int_EnableConfig = INT_ON;

        boot_Set6MHzModeI(); // Enter low-power mode

        int_Wait(); // Wait for user to press On

        boot_Set48MHzModeI(); // Leave low-power mode

        // Restore and disable interrupts
        int_Disable();
        kb_ClearOnLatch();
        int_EnableConfig = enableConfigBak;
    }

    boot_TurnOn();
    sh_init();
}