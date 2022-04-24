////////////////////////////////////////////////////////////////
// { CEsh } { v0.1a }                                         //
// Author: calclover2514                                      //
// License: GPL v3                                            //
// Description: A (ba)sh-inspired shell for the TI-84 Plus CE //
////////////////////////////////////////////////////////////////
#include <debug.h>

/* Keep these headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

/* Standard headers */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Library headers */
#include <graphx.h>
#include <fontlibc.h>
#include <fileioc.h>
#include <keypadc.h>

/* Include converted data */
#include "gfx/gfx.h"

/* Variable definitions */
#include "include/globals.h"
#include "include/macros.h"
#include "include/types.h"

/* Function definitions */
#include "include/input.h"
#include "include/draw.h"
#include "include/routines.h"


/* Function declarations */
void cesh_Init(void);
void cesh_Setup(void);
void cesh_Splash(void);
void cesh_Shell(void);
void cesh_PreGC(void);
void cesh_End(void);


int main(void)
{
    cesh_Init();
    cesh_Shell();
    cesh_End();
    return 0;
}


// Initialize program
void cesh_Init(void) {

    uint8_t i, j;
    uint16_t cursorPos[2] = {SCR_OFFSET_X, SCR_OFFSET_Y};
    uint8_t textColors[2] = {WHITE, BLACK};
    bool sdUnderlineText = false,
         sdItalicText = false,
         sdBoldText = false;

    // Empty user/pwd variables
    for (i = 0; i < USER_PWD_LENGTH; i++) {
        user[i] = 0;
        pwd[i] = 0;
    }

    // Empty screen buffer
    for (i = 0; i < SCR_WIDTH; i++) {
        for (j = 0; j < SCR_HEIGHT; j++) {
            scrBuffer[j][i].character = ' ';
            scrBuffer[j][i].bold = false;
            scrBuffer[j][i].italic = false;
            scrBuffer[j][i].underline = false;
            scrBuffer[j][i].fg_col = WHITE;
            scrBuffer[j][i].bg_col = BLACK;
        }
    }

    ti_SetGCBehavior(cesh_PreGC, cesh_Init);

    kb_EnableOnLatch();

    gfx_Begin();
    gfx_SetPalette(imgPalette, sizeof_imgPalette, 0);
    gfx_SetDrawBuffer();

    // Set up font
    fontlib_SetFont(terminus_font, 0);
    fontlib_SetTransparency(false);
    fontlib_SetLineSpacing(0, 0);
    fontlib_SetWindow(SCR_OFFSET_X, SCR_OFFSET_Y, SCR_WIDTH_P, SCR_HEIGHT_P);
    fontlib_SetNewlineOptions(FONTLIB_ENABLE_AUTO_WRAP);// | FONTLIB_AUTO_SCROLL);
    fontlib_ClearWindow();

    gfx_FillScreen(BLACK);
    gfx_BlitBuffer();

    // Load shell state
    appvarSlot = ti_Open("CEshSett", "r");

    if (appvarSlot != 0) {
        ti_Read(&user, sizeof(char), USER_PWD_LENGTH, appvarSlot);
        ti_Read(&pwd, sizeof(char), USER_PWD_LENGTH, appvarSlot);
        ti_Read(&isRetFromPrgm, sizeof(bool), 1, appvarSlot);
    } else {
        settingsAppvarExists = false;
        appvarSlot = ti_Open("CEshSett", "w+");
        ti_Write(&user, sizeof(char), USER_PWD_LENGTH, appvarSlot);
        ti_Write(&pwd, sizeof(char), USER_PWD_LENGTH, appvarSlot);
        ti_Write(&isRetFromPrgm, sizeof(bool), 1, appvarSlot);
        ti_Write(&cursorPos, sizeof(uint16_t), 2, appvarSlot);
        ti_Write(&textColors, sizeof(uint8_t), 2, appvarSlot);
        ti_Write(&dateTime, sizeof(uint16_t), 7, appvarSlot);
        ti_Write(&sdUnderlineText, sizeof(bool), 1, appvarSlot);
        ti_Write(&sdItalicText, sizeof(bool), 1, appvarSlot);
        ti_Write(&sdBoldText, sizeof(bool), 1, appvarSlot);
        ti_SetArchiveStatus(true, appvarSlot);
    }

    if (isRetFromPrgm) {
        ti_Read(&cursorPos, sizeof(uint16_t), 2, appvarSlot);
        ti_Read(&textColors, sizeof(uint8_t), 2, appvarSlot);
        ti_Seek(sizeof(uint16_t) * 7, SEEK_SET, appvarSlot);
        ti_Read(&sdUnderlineText, sizeof(bool), 1, appvarSlot);
        ti_Read(&sdItalicText, sizeof(bool), 1, appvarSlot);
        ti_Read(&sdBoldText, sizeof(bool), 1, appvarSlot);
    }

    ti_Close(appvarSlot);

    // Attempt to load screen state
    appvarSlot = ti_Open("CEshSBuf", "r");
    if (appvarSlot != 0) {
        ti_Read(&scrBuffer, sizeof(char_styled_t), BUFFER_SIZE, appvarSlot);
        ti_Close(appvarSlot);
        ti_Delete("CEshSBuf");

        for (i = 0; i < SCR_WIDTH; i++) {
            for (j = 0; j < SCR_HEIGHT; j++) {
                if (underlineText || italicText || boldText) parse_draw_string("\\e[0m");
                if (scrBuffer[j][i].bold) parse_draw_string("\\e[1m");
                if (scrBuffer[j][i].italic) parse_draw_string("\\e[3m");
                if (scrBuffer[j][i].underline) parse_draw_string("\\e[4m");
                fontlib_SetColors(scrBuffer[j][i].fg_col, scrBuffer[j][i].bg_col);
                fontlib_SetCursorPosition(SCR_OFFSET_X + (i * FONT_WIDTH), SCR_OFFSET_Y + (j * FONT_HEIGHT));
                fontlib_DrawGlyph(scrBuffer[j][i].character);
                if (underlineText) {
                    gfx_SetColor(fontlib_GetForegroundColor());
                    gfx_HorizLine(fontlib_GetCursorX() - FONT_WIDTH, fontlib_GetCursorY() + FONT_HEIGHT - 2, FONT_WIDTH);
                }
            }
        }
        parse_draw_string("\\e[0m");
        if (sdBoldText) {
            parse_draw_string("\\e[1m");
        }
        if (sdItalicText) {
            parse_draw_string("\\e[3m");
        }
        if (sdUnderlineText) {
            parse_draw_string("\\e[4m");
        }
    }

    // Set colors
    gfx_SetTextBGColor(textColors[1]);
    gfx_SetTextFGColor(textColors[0]);
    gfx_SetColor(textColors[0]);
    fontlib_SetColors(textColors[0], textColors[1]);

    fontlib_SetCursorPosition(cursorPos[0], cursorPos[1]);

    strcpy(path, "/");
}

// First time setup
void cesh_Setup(void) {

    uint8_t i;
    char pwd_tmp[USER_PWD_LENGTH];

    // Get new username
    draw_newline();
    draw_str_update_buf("Please create a default CEsh user account.");
    draw_newline();
    get_user_input("Enter new CEsh username: ", false, true, 0);

    for (i = 0; i < USER_PWD_LENGTH - 1; i++) {
        user[i] = input[i];
    }

    do {
        // Get new password
        draw_newline();
        get_user_input("New password: ", true, true, 0);

        for (i = 0; i < USER_PWD_LENGTH - 1; i++) {
            pwd_tmp[i] = input[i];
        }

        // Confirm new password
        draw_newline();
        get_user_input("Confirm password: ", true, true, 0);
        draw_newline();

        for (i = 0; i < USER_PWD_LENGTH - 1; i++) {
            pwd[i] = input[i];
        }

        if (strcmp(pwd_tmp, pwd)) {
            draw_str_update_buf("Passwords do not match!");
        }

    } while (strcmp(pwd_tmp, pwd));

    // Store in appvar
    appvarSlot = ti_Open("CEshSett", "r+");
    ti_Write(&user, sizeof(char), USER_PWD_LENGTH, appvarSlot);
    ti_Write(&pwd, sizeof(char), USER_PWD_LENGTH, appvarSlot);
    ti_SetArchiveStatus(true, appvarSlot);
    ti_Close(appvarSlot);

    settingsAppvarExists = true;

    draw_str_update_buf("User account created successfully!");
    draw_newline();
    draw_newline();
}

// Splash screen
void cesh_Splash(void) {

    uint8_t i, sec, min, hr;

    boot_GetTime(&sec, &min, &hr);
    srandom(sec + (min * 60) + (hr * 360));

    gfx_FillScreen(BLACK);
    gfx_Sprite(imgSplash, (LCD_WIDTH - imgSplash_width) / 2, (LCD_HEIGHT - imgSplash_height) * 5 / 12);
    for (i = 0; i < (random() % 15) + 3; i++) {
        gfx_SetColor(inRange((i % 6),1,3) ? WHITE : BLACK);
        gfx_FillCircle((LCD_WIDTH / 2) - 18, LCD_HEIGHT * 2 / 3, 3);
        gfx_SetColor(inRange((i % 6),2,4) ? WHITE : BLACK);
        gfx_FillCircle(LCD_WIDTH / 2, LCD_HEIGHT * 2 / 3, 3);
        gfx_SetColor(inRange((i % 6),3,5) ? WHITE : BLACK);
        gfx_FillCircle((LCD_WIDTH / 2) + 18, LCD_HEIGHT * 2 / 3, 3);
        gfx_BlitBuffer();
        delay(400);
    }
    gfx_FillScreen(BLACK);
}

// Main shell loop
void cesh_Shell(void) {

    uint8_t temp, i, day, mon, sec, min, hr;
    uint16_t yr,
             dateTimeTemp[7];
    bool fts = false,
         lastLoginHappened = false;

    if (!isRetFromPrgm) {

        if (noSplash) {
            noSplash = false;
        } else {
            cesh_Splash();
        }
        
        // Startup text
        draw_str_update_buf("CEsh v0.1a - The TI-84 Plus CE terminal");
        draw_newline();
        temp = fontlib_GetForegroundColor();
        fontlib_SetForegroundColor(BRIGHT_BLACK);
        draw_str_update_buf("(Press [2nd]+[mode] to exit)");
        fontlib_SetForegroundColor(temp);
        draw_newline();
        draw_newline();

        if (!settingsAppvarExists) {
            cesh_Setup();
            fts = true;
        }

        temp = fontlib_GetCursorY();

        // Login & password prompt
        do {
            gfx_SetColor(fontlib_GetBackgroundColor());
            gfx_FillRectangle(SCR_OFFSET_X, temp, SCR_WIDTH_P, SCR_HEIGHT_P - temp);
            fontlib_SetCursorPosition(SCR_OFFSET_X, temp);
            draw_str_update_buf(CALC_NAME);
            get_user_input(" login: ", false, true, (strlen(CALC_NAME) * FONT_WIDTH) + SCR_OFFSET_X);
            input[USER_PWD_LENGTH] = 0;
        } while (strcmp(input, user));

        draw_newline();
        temp = fontlib_GetCursorY();

        do {
            gfx_SetColor(fontlib_GetBackgroundColor());
            gfx_FillRectangle(SCR_OFFSET_X, temp, SCR_WIDTH_P, SCR_HEIGHT_P - temp);
            fontlib_SetCursorPosition(SCR_OFFSET_X, temp);
            get_user_input("Password: ", true, true, 0);
            input[USER_PWD_LENGTH] = 0;
        } while (strcmp(input, pwd));

        // Store date and time
        boot_GetDate(&day, &mon, &yr);
        boot_GetTime(&sec, &min, &hr);
        dateTime[0] = yr;
        dateTime[1] = (uint16_t)mon;
        dateTime[2] = (uint16_t)day;
        dateTime[3] = (uint16_t)hr;
        dateTime[4] = (uint16_t)min;
        dateTime[5] = (uint16_t)sec;

        // Get day of week
        dateTime[6] = getDayOfWeek(yr,mon,day);

        appvarSlot = ti_Open("CEshSett", "r+");
        ti_Seek((2 * USER_PWD_LENGTH) + 7, SEEK_SET, appvarSlot); // Go to correct location in settings appvar

        // Read last login date
        if (fts) {
            for (i = 0; i < 7; i++) {
                dateTimeTemp[i] = dateTime[i];
            }
            fts = false;
        } else {
            ti_Read(&dateTimeTemp, sizeof(uint16_t), 7, appvarSlot);
            ti_Seek(sizeof(uint16_t) * -7, SEEK_CUR, appvarSlot);
            lastLoginHappened = true;
        }

        ti_Write(&dateTime, sizeof(uint16_t), 7, appvarSlot); // Write current login date

        ti_SetArchiveStatus(true, appvarSlot);
        ti_Close(appvarSlot);

        // Startup info
        if (lastLoginHappened) {
            draw_newline();
            draw_str_update_buf("Last login: ");
            draw_str_update_buf(DOW_NAMES[dateTimeTemp[6]]);
            draw_str_update_buf(" ");
            draw_str_update_buf(MON_NAMES[dateTimeTemp[1] - 1]);
            draw_str_update_buf(" ");
            draw_int_update_buf(dateTimeTemp[2], 2);
            draw_str_update_buf(" ");
            draw_int_update_buf(dateTimeTemp[3], 2);
            draw_str_update_buf(":");
            draw_int_update_buf(dateTimeTemp[4], 2);
            draw_str_update_buf(":");
            draw_int_update_buf(dateTimeTemp[5], 2);
            draw_str_update_buf(" ");
            draw_int_update_buf(dateTimeTemp[0], 4);
            draw_newline();
        }

        draw_newline();
        draw_str_update_buf("The programs included with the CE shell are free");
        draw_newline();
        draw_str_update_buf("software; the exact distribution terms for each");
        draw_newline();
        draw_str_update_buf("program are described in their respective README");
        draw_newline();
        draw_str_update_buf("files.");
        draw_newline();
        draw_newline();
        draw_str_update_buf("CEsh comes with ABSOLUTELY NO WARRANTY, to the");
        draw_newline();
        draw_str_update_buf("extent permitted by applicable law.");
        draw_newline();
    }

    // Main loop
    do {
        // Decide whether to start the prompt on a new line or not
        if (startOnNewLine) {
            draw_newline();
        } else {
            startOnNewLine = true;
        }

        // Display input prompt
        temp = fontlib_GetForegroundColor();
        fontlib_SetForegroundColor(GREEN);
        draw_str_update_buf(user);
        draw_str_update_buf("@");
        draw_str_update_buf(CALC_NAME);
        fontlib_SetForegroundColor(temp);
        draw_str_update_buf(":");
        fontlib_SetForegroundColor(BRIGHT_BLUE);
        draw_str_update_buf(path);
        fontlib_SetForegroundColor(temp);

        get_user_input("$ ", false, false, fontlib_GetCursorX());
        if (strlen(input)) parse_user_input();

    } while (strcmp(input, "exit")); // Keep going until user types exit
}

// Handle GarbageCollect
void cesh_PreGC(void) {

    // Save screen state
    appvarSlot = ti_Open("CEshSBuf", "w+");
    ti_Write(&scrBuffer, sizeof(char_styled_t), BUFFER_SIZE, appvarSlot);
    ti_Close(appvarSlot);

    gfx_End();
}

// End program
void cesh_End(void) {
    gfx_End();

    // Mark shell as exited
    isRetFromPrgm = false;
    appvarSlot = ti_Open("CEshSett", "r+");
    ti_Seek(2 * USER_PWD_LENGTH, SEEK_SET, appvarSlot);
    ti_Write(&isRetFromPrgm, sizeof(bool), 1, appvarSlot);
    ti_SetArchiveStatus(true, appvarSlot);
    ti_Close(appvarSlot);
    appvarSlot = ti_Open("CEshHist", "r");
    ti_SetArchiveStatus(true, appvarSlot);
    ti_Close(appvarSlot);

    exit(0);
}