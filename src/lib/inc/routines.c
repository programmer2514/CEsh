#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <graphx.h>
#include <fontlibc.h>
#include <fileioc.h>
#include <keypadc.h>

#include "globals.h"
#include "macros.h"
#include "types.h"
#include "input.h"

#include "../cesh.h"

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

int run_prgm(char *prgm, char *args) {

    int ret;
    uint16_t cursorPos[2] = {SCR_OFFSET_X, SCR_OFFSET_Y};
    uint8_t textColors[2] = {WHITE, BLACK};
    bool sdUnderlineText = false,
         sdItalicText = false,
         sdBoldText = false;

    // Make sure program actually exists
    appvarSlot = ti_OpenVar(prgm, "r", TI_PRGM_TYPE);
    if (appvarSlot == 0) {
        return -1;
    }
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

    // Save screen state
    appvarSlot = ti_Open("CEshSBuf", "w+");
    ti_Write(&scrBuffer, sizeof(char_styled_t), BUFFER_SIZE, appvarSlot);
    ti_Close(appvarSlot);

    gfx_End();

    ret = os_RunPrgm(prgm, NULL, 0, (os_runprgm_callback_t)cesh_Main);
    cesh_Init();

    return ret;
}

void power_down(bool restart, bool save) {

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
    while ((!kb_On) && (!restart)); // Wait for user to press On
    boot_TurnOn();

    kb_ClearOnLatch();

    cesh_Init();
}