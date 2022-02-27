////////////////////////////////////////////////////////////////
// { CEsh } { v0.1a }                                         //
// Author: calclover2514                                      //
// License: GPL v3                                            //
// Description: A (ba)sh-inspired shell for the TI-84 Plus CE //
////////////////////////////////////////////////////////////////


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
#include "fonts/fonts.h"


/* Structure definitions */
typedef struct __char_styled__ {
    char character : 8;
    bool bold      : 1;
    bool italic    : 1;
    bool underline : 1;
    uint8_t fg_col : 4;
    uint8_t bg_col : 4;
} char_styled_t;


/* Function declarations */
void cesh_Init(void);
void cesh_Setup(void);
void cesh_Shell(void);
void cesh_End(void);
void parse_user_input(void);
void get_user_input(const char *msg, const bool maskInput, const uint16_t offsetX);
void parse_draw_string(const char *string, const bool justDraw);
void draw_str_update_buf(const char *string);
void draw_int_update_buf(int number, const uint8_t length);
void draw_newline(void);
uint8_t str_to_num(const char *string, const uint8_t length, const uint8_t base);
int run_prgm(char *prgm, char *args);


/* Variable declarations */
const char *pCalcName = (char *)0x3B000E;
const char cursors[4] = {129, 130, 128, 95};
const char dowNames[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char monNames[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

char input[257];
char parserData[257];
char path[257];
char calcName[25];
uint8_t textIndex = 0;

uint16_t cursorPos[2] = {2, 2};
bool startOnNewLine = true;
bool underlineText = false;
bool italicText = false;
bool boldText = false;

char user[33];
char pwd[33];
bool sdRetFromPrgm = false;
uint16_t sdCursorPos[2] = {2, 2};
uint8_t sdTextColors[2] = {7, 0};

gfx_sprite_t *sdScr;
ti_var_t settingsAppvar;
bool settingsAppvarExists = true;

uint16_t dateTime[7];
bool lastLoginHappened = false;

char_styled_t scrBuffer[52][19];


int main(void)
{
    cesh_Init();
    cesh_Shell();
    cesh_End();
    return 0;
}


/* Functions */

// Initialize program
void cesh_Init(void) {
    
    uint8_t i, j;
    
    sdScr = gfx_MallocSprite(160, 126); // Initialize sprite variable
    
    // Empty user/pwd variables
    for (i = 0; i <= 32; i++) {
        user[i] = 0;
        pwd[i] = 0;
    }
    
    // Empty screen buffer
    for (i = 0; i < 52; i++) {
        for (j = 0; j < 19; j++) {
            scrBuffer[i][j].character = ' ';
            scrBuffer[i][j].bold = false;
            scrBuffer[i][j].italic = false;
            scrBuffer[i][j].underline = false;
            scrBuffer[i][j].fg_col = 7;
            scrBuffer[i][j].bg_col = 0;
        }
    }

    ti_SetGCBehavior(gfx_End, cesh_Init);
    
    gfx_Begin();
    gfx_SetPalette(imgPalette, sizeof_imgPalette, 0);
    gfx_SetDrawBuffer();

    // Set up font
    fontlib_SetFont(terminus_font, 0);
    fontlib_SetTransparency(false);
    fontlib_SetLineSpacing(0, 0);
    fontlib_SetWindow(2, 2, 316, 236);
    fontlib_SetNewlineOptions(FONTLIB_ENABLE_AUTO_WRAP);// | FONTLIB_AUTO_SCROLL);
    fontlib_ClearWindow();

    gfx_FillScreen(0);
    
    // Attempt to load screen state
    settingsAppvar = ti_Open("CEshScrA", "r");
    if (settingsAppvar != 0) {
        ti_Read(&sdScr, sizeof(char), 20162, settingsAppvar);
        gfx_Sprite(sdScr, 0, 0);
        ti_Read(&sdScr, sizeof(char), 20162, settingsAppvar);
        gfx_Sprite(sdScr, 160, 0);
        
        ti_Close(settingsAppvar);
        ti_Delete("CEshScrA");
    }
    
    settingsAppvar = ti_Open("CEshScrB", "r");
    if (settingsAppvar != 0) {
        ti_Read(&sdScr, sizeof(char), 20162, settingsAppvar);
        gfx_Sprite(sdScr, 0, 120);
        ti_Read(&sdScr, sizeof(char), 20162, settingsAppvar);
        gfx_Sprite(sdScr, 160, 120);
        
        ti_Close(settingsAppvar);
        ti_Delete("CEshScrB");
        
    }
    
    // Load shell state
    settingsAppvar = ti_Open("CEshSett", "r");
    
    if (settingsAppvar != 0) {
        ti_Read(&user, sizeof(char), 33, settingsAppvar);
        ti_Read(&pwd, sizeof(char), 33, settingsAppvar);
        ti_Read(&sdRetFromPrgm, sizeof(bool), 1, settingsAppvar);
    } else {
        settingsAppvarExists = false;
        settingsAppvar = ti_Open("CEshSett", "w+");
        ti_Write(&user, sizeof(char), 33, settingsAppvar);
        ti_Write(&pwd, sizeof(char), 33, settingsAppvar);
        ti_Write(&sdRetFromPrgm, sizeof(bool), 1, settingsAppvar);
        ti_Write(&sdCursorPos, sizeof(uint16_t), 2, settingsAppvar);
        ti_Write(&sdTextColors, sizeof(uint8_t), 2, settingsAppvar);
        ti_Write(&dateTime, sizeof(uint16_t), 7, settingsAppvar);
    }
    
    if (sdRetFromPrgm) {
        ti_Read(&sdCursorPos, sizeof(uint16_t), 2, settingsAppvar);
        ti_Read(&sdTextColors, sizeof(uint8_t), 2, settingsAppvar);
    }
        
    ti_SetArchiveStatus(true, settingsAppvar);
    ti_Close(settingsAppvar);
    
    // Set colors
    gfx_SetTextBGColor(sdTextColors[1]);
    gfx_SetTextFGColor(sdTextColors[0]);
    gfx_SetColor(sdTextColors[0]);
    fontlib_SetColors(sdTextColors[0], sdTextColors[1]);
    
    fontlib_SetCursorPosition(sdCursorPos[0], sdCursorPos[1]);

    strcpy(path, "/");
    strcpy(calcName, pCalcName);
}

// First time setup
void cesh_Setup(void) {
    
    uint8_t i;
    char pwd_tmp[33];
    
    // Get new username
    draw_newline();
    draw_str_update_buf("Please create a default CEsh user account.");
    draw_newline();
    get_user_input("Enter new CEsh username: ", false, 0);
    
    for (i = 0; i <= 32; i++) {
        user[i] = input[i];
    }
    
    do {
        // Get new password
        draw_newline();
        get_user_input("New password: ", true, 0);
    
        for (i = 0; i <= 32; i++) {
            pwd_tmp[i] = input[i];
        }
    
        // Confirm new password
        draw_newline();
        get_user_input("Confirm password: ", true, 0);
        draw_newline();
    
        for (i = 0; i <= 32; i++) {
            pwd[i] = input[i];
        }
        
        if (strcmp(pwd_tmp, pwd)) {
            draw_str_update_buf("Passwords do not match!");
        }
        
    } while (strcmp(pwd_tmp, pwd));
    
    // Store in appvar
    settingsAppvar = ti_Open("CEshSett", "r+");
    ti_Write(&user, sizeof(char), 33, settingsAppvar);
    ti_Write(&pwd, sizeof(char), 33, settingsAppvar);
    ti_SetArchiveStatus(true, settingsAppvar);
    ti_Close(settingsAppvar);
    
    settingsAppvarExists = true;
    
    draw_str_update_buf("User account created successfully!");
    draw_newline();
    draw_newline();
}

// Main shell loop
void cesh_Shell(void) {

    uint8_t temp;
    uint8_t i;
    uint8_t day, mon, sec, min, hr, dow;
    uint16_t yr;
    uint16_t dateTimeTemp[7];
    bool fts = false;

    if (!sdRetFromPrgm) {
        // Startup text
        draw_str_update_buf("CEsh v0.1a - The TI-84 Plus CE terminal");
        draw_newline();
        temp = fontlib_GetForegroundColor();
        fontlib_SetForegroundColor(0x08);
        draw_str_update_buf("(Press [2nd]+[mode] to exit)");
        fontlib_SetForegroundColor(temp);
        draw_newline();
        draw_newline();
    
        if (!settingsAppvarExists) {
            cesh_Setup();
            fts = true;
        }
        
        // Login & password prompt
        do {
            draw_str_update_buf(calcName);
            get_user_input(" login: ", false, (strlen(calcName) * 6) + 2);
        } while (strcmp(input, user));
        
        draw_newline();
        
        do {
            get_user_input("Password: ", true, 0);
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
        dow = (day += mon < 3 ? yr-- : yr - 2, 23*mon/9 + day + 4 + yr/4- yr/100 + yr/400)%7;
        dateTime[6] = (uint16_t)dow;
        
        settingsAppvar = ti_Open("CEshSett", "r+");
        ti_Seek(73, SEEK_SET, settingsAppvar); // Go to correct location in settings appvar
    
        // Read last login date
        if (fts) {
            for (i = 0; i < 7; i++) {
                dateTimeTemp[i] = dateTime[i];
            }
            fts = false;
        } else {
            ti_Read(&dateTimeTemp, sizeof(uint16_t), 7, settingsAppvar);
            ti_Seek(sizeof(uint16_t) * -7, SEEK_CUR, settingsAppvar);
            lastLoginHappened = true;
        }
        
        ti_Write(&dateTime, sizeof(uint16_t), 7, settingsAppvar); // Write current login date
        
        ti_SetArchiveStatus(true, settingsAppvar);
        ti_Close(settingsAppvar);
    
        // Startup info
        if (lastLoginHappened) {
            draw_newline();
            draw_str_update_buf("Last login: ");
            draw_str_update_buf(dowNames[dateTimeTemp[6]]);
            draw_str_update_buf(" ");
            draw_str_update_buf(monNames[dateTimeTemp[1] - 1]);
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
        fontlib_SetForegroundColor(0x02);
        parse_draw_string(user, true);
        parse_draw_string("@", true);
        parse_draw_string(calcName, true);
        fontlib_SetForegroundColor(temp);
        parse_draw_string(":", true);
        fontlib_SetForegroundColor(0x0C);
        parse_draw_string(path, true);
        fontlib_SetForegroundColor(temp);

        get_user_input("$ ", false, fontlib_GetCursorX());
        parse_user_input();

    } while (strcmp(input, "exit")); // Keep going until user types exit
}

// End program
void cesh_End(void) {
    gfx_End();
    
    // Mark shell as exited
    sdRetFromPrgm = false;
    settingsAppvar = ti_Open("CEshSett", "r+");
    ti_Seek(sizeof(char) * 66, SEEK_SET, settingsAppvar);
    ti_Write(&sdRetFromPrgm, sizeof(bool), 1, settingsAppvar);
    ti_SetArchiveStatus(true, settingsAppvar);
    ti_Close(settingsAppvar);
    
    exit(0);
}

// Parse user input
void parse_user_input(void) {

    uint16_t i, j;
    char *ptr;
    int retval;
    uint8_t numargs = 1;
    uint8_t *arglocs = malloc(1);
    bool inQuotes = false;
    
    arglocs[0] = 0; // Pre-fill location of command name, as it's always 0
    
    // Pre-search input to define argument delimiters and remove quotes
    for (i = 0; i < 256; i++) {
        if ((input[i] == '"') || (input[i] == '\'')) {
            if (i > 0) {
                if (input[i - 1] != '\\') {
                    inQuotes = !inQuotes;
                    input[i] = 2;
                }
            } else {
                inQuotes = !inQuotes;
                input[i] = 2;
            }
        } else if ((input[i] == ' ') && (!inQuotes)) {
            input[i] = 2;
        }
    }
    
    // Split input into args
    ptr = strtok(input, "\2");
    while (ptr != NULL) {
		ptr = strtok(NULL, "\2");

        if (ptr != NULL) {
            // Add location of next arg to end of arglocs
            numargs++;
            arglocs = (uint8_t *) realloc(arglocs, numargs);
            arglocs[numargs - 1] = ptr - input;
        }
	}

    /* Parse commands */

    // Command: echo
    if (!strcmp(input, "echo")) {
        
        startOnNewLine = true;
        draw_newline();
        
        for (i = 1; i < numargs; i++) {
            parse_draw_string(&input[arglocs[i]], false);
            if (i < numargs - 1) {
                parse_draw_string(" ", true);
            }
        }
        
    // Prevent shell running itself
    } else if (!strcmp(input, "./CESH")) {
        
        draw_newline();
        parse_draw_string("No", true);
        
    // Debug command (remove in release)
    } else if (!strcmp(input, "dbg")) {
        
        startOnNewLine = true;
        draw_newline();
        for (i = 0; i < numargs; i++) {
            draw_int_update_buf(i, 1);
            parse_draw_string(": ", true);
            parse_draw_string(&input[arglocs[i]], true);
            parse_draw_string("\\n", false);
        }
        
        /* for (i = 0; i < 52; i++) {
            for (j = 0; j < 19; j++) {
                if (scrBuffer[i][j].bold) {
                    parse_draw_string("\\e[1m", false);
                }
                if (scrBuffer[i][j].italic) {
                    parse_draw_string("\\e[3m", false);
                }
                if (scrBuffer[i][j].underline) {
                    parse_draw_string("\\e[4m", false);
                }
                fontlib_SetColors(scrBuffer[i][j].fg_col, scrBuffer[i][j].bg_col);
                fontlib_SetCursorPosition(2 + (i * 6), 2 + (j * 12));
                fontlib_DrawGlyph(scrBuffer[i][j].character);
            }
        } */
        
    // Command: .
    } else if (input[0] == '.') {
        
        if (input[1] == '/') {
            retval = run_prgm(&input[2], "null");
            
            draw_newline();
            parse_draw_string("Error ", true);
            draw_int_update_buf(retval, 2);
            parse_draw_string(": ", true);
            if (retval == -1) {
                parse_draw_string(input, true);
                parse_draw_string(": No such file or directory", true);
            } else if (retval == -2) {
                parse_draw_string(input, true);
                parse_draw_string(": Not enough memory", true);
            } else {
                parse_draw_string(input, true);
                parse_draw_string(": Unspecified error", true);
            }
        } else {
            parse_draw_string(".: usage: ./filename [arguments]", true);
        }

    // If not a built-in command and not empty
    } else if (strcmp(input, "") && strcmp(input, "exit") && (input[0] != ' ') && (input[0] != 0)) {

        draw_newline();
        parse_draw_string(input, true);
        parse_draw_string(": command not found", true);

    }

    gfx_BlitBuffer();
}

// Get user input
void get_user_input(const char *msg, const bool maskInput, const uint16_t offsetX) {

    bool key = false, prevkey = true, done = false;
    uint16_t i, cursorY = fontlib_GetCursorY();
    int16_t j = 0, k, lineWrap;
    char temp[2] = {0, 0};

    // Empty output
    for (i = 0; i <= 256; i++) {
        input[i] = 0;
    }

    textIndex = 3; // Reset cursor state

    lineWrap = offsetX + 6; // Initialize lineWrap with current X offset + cursor width

    gfx_BlitBuffer();

    do {
        // Update keyboard data
        kb_Scan();
        key = kb_Data[1] || kb_Data[2] || kb_Data[3] || kb_Data[4] || kb_Data[5] || kb_Data[6] || kb_Data[7];

        // Blink Cursor
        if (j == 0) { // Update screen output w/o cursor
        
            // Loop backwards through wrapped lines and clear everything back to original X offset until redraw
            for (i = fontlib_GetCursorY(); i >= cursorY; i = i - 12) {
                fontlib_Home();
                if ((i == cursorY) && offsetX) {
                    fontlib_SetCursorPosition(offsetX, fontlib_GetCursorY());
                }
                fontlib_ClearEOL();
                fontlib_SetCursorPosition(fontlib_GetCursorX(), i);
            }

            parse_draw_string(msg, true);

            // Draw stars if masking input, otherwise output plain text
            if (maskInput) {
                for (i = 1; i <= (int16_t)strlen(input); i++) {
                    parse_draw_string("*", true);
                }
            } else {
                parse_draw_string(input, true);
            }

            // Redraw cursor
            temp[0] = cursors[textIndex];
            parse_draw_string(temp, true);

            gfx_BlitBuffer();

        } else if (j == 500) { // Update screen output w/ cursor

            // Loop backwards through wrapped lines and clear everything back to original X offset until redraw
            for (i = fontlib_GetCursorY(); i >= cursorY; i = i - 12) {
                fontlib_Home();
                if ((i == cursorY) && offsetX) {
                    fontlib_SetCursorPosition(offsetX, fontlib_GetCursorY());
                }
                fontlib_ClearEOL();
                fontlib_SetCursorPosition(fontlib_GetCursorX(), i);
            }

            parse_draw_string(msg, true);

            // Draw stars if masking input, otherwise output plain text
            if (maskInput) {
                for (i = 1; i <= (int16_t)strlen(input); i++) {
                    parse_draw_string("*", true);
                }
            } else {
                parse_draw_string(input, true);
            }
            
            gfx_BlitBuffer();
        
        } else if (j == 1000) { // Reset j once cursor has flashed one time
            j = -1;
        }

        j++;

        // Update output
        if (key && !prevkey) {
            
            switch (kb_Data[1]) {
                case kb_2nd:
                    if ((textIndex == 2) || (textIndex == 3)) {
                        textIndex = 0;
                    } else if (textIndex == 0) {
                        textIndex = 1;
                    } else if (textIndex == 1) {
                        textIndex = 3;
                    }
                    break;
                case kb_Mode:
                    if (textIndex < 2) {
                        cesh_End();
                    }
                    break;
                case kb_Del:
                    if (strlen(input)) {
                        input[strlen(input)-1] = 0;
                        lineWrap = lineWrap - 6;
                    }
                    break;
                default:
                    break;
            }

            switch (kb_Data[2]) {
                case kb_Sto:
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'X';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'x';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_Ln:
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'S';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 's';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_Log:
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'N';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'n';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_Square:
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'I';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'i';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_Recip:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = '\\';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'D';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'd';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_Math:
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'A';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'a';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_Alpha:
                    if (textIndex != 2) {
                        textIndex = 2;
                    } else {
                        textIndex = 3;
                    }
                    break;
                default:
                    break;
            }

            switch (kb_Data[3]) {
                case kb_0:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = '0';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) || (textIndex == 3)) {
                        input[strlen(input)] = ' ';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_1:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = '1';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'Y';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'y';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_4:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = '4';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'T';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 't';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_7:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = '7';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'O';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'o';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_Comma:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = ',';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 1) && (strlen(input) < 255)) {
                        input[strlen(input)] = '=';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'J';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'j';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_Sin:
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'E';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'e';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_Apps:
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'B';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'b';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                default:
                    break;
            }

            switch (kb_Data[4]) {
                case kb_DecPnt:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = '.';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 1) && (strlen(input) < 255)) {
                        input[strlen(input)] = '!';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = ':';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = ';';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_2:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = '2';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'Z';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'z';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_5:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = '5';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'U';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'u';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_8:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = '8';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'P';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'p';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_LParen:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = '(';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 1) && (strlen(input) < 255)) {
                        input[strlen(input)] = '{';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'K';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'k';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_Cos:
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'F';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'f';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_Prgm:
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'C';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'c';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                default:
                    break;
            }

            switch (kb_Data[5]) {
                case kb_Chs:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = '_';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) || (textIndex == 3)) {
                        input[strlen(input)] = '?';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_3:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = '3';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) || (textIndex == 3)) {
                        input[strlen(input)] = '@';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_6:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = '6';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'V';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'v';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_9:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = '9';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'Q';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'q';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_RParen:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = ')';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 1) && (strlen(input) < 255)) {
                        input[strlen(input)] = '}';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'L';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'l';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_Tan:
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'G';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'g';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                default:
                    break;
            }

            switch (kb_Data[6]) {
                case kb_Enter:
                    done = true;
                    break;
                case kb_Add:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = '+';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = '\'';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = '"';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_Sub:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = '-';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 1) && (strlen(input) < 255)) {
                        input[strlen(input)] = ']';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'W';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'w';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_Mul:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = '*';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 1) && (strlen(input) < 255)) {
                        input[strlen(input)] = '[';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'R';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'r';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_Div:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = '/';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 1) && (strlen(input) < 255)) {
                        input[strlen(input)] = '&';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'M';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'm';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_Power:
                    if ((textIndex == 0) && (strlen(input) < 255)) {
                        input[strlen(input)] = '^';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 1) && (strlen(input) < 255)) {
                        input[strlen(input)] = '|';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 2) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'H';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    if ((textIndex == 3) && (strlen(input) < 255)) {
                        input[strlen(input)] = 'h';
                        input[strlen(input)+1] = 0;
                        lineWrap = lineWrap + 6;
                    }
                    break;
                case kb_Clear:
                    for (i = 0; i <= 256; i++) {
                        input[i] = 0;
                    }
                    lineWrap = offsetX + 6;
                    break;
                default:
                    break;
            }

            switch (kb_Data[7]) {
                case kb_Down:
                    break;
                case kb_Up:
                    break;
                default:
                    break;
            }

            /* Immediately update screen output when input string changed */

            // Update lineWrap and cursorY (prevents fontlib's autoscroll)
            if ((lineWrap >= 308) && (fontlib_GetCursorY() >= 218)) {
                lineWrap = 0;
                cursorY = cursorY - 12;
                fontlib_ScrollWindowDown(); // Manually scroll window
                
                // Scroll the contents of the screen buffer
                memmove(&scrBuffer[0][0], &scrBuffer[0][1], 936 * sizeof(char_styled_t));
                for (k = 0; k < 52; k++) {
                    scrBuffer[k][18].character = ' ';
                    scrBuffer[k][18].bold = false;
                    scrBuffer[k][18].italic = false;
                    scrBuffer[k][18].underline = false;
                    scrBuffer[k][18].fg_col = 7;
                    scrBuffer[k][18].bg_col = 0;
                }
                
                // Erase the bottom line
                gfx_SetColor(0);
                gfx_FillRectangle(0, 218, 320, 22);
            }

            if (lineWrap >= 308) lineWrap = 0;

            // Loop backwards through wrapped lines and clear everything back to original X offset until redraw
            for (i = fontlib_GetCursorY(); i >= cursorY; i = i - 12) {
                fontlib_Home();
                if ((i == cursorY) && offsetX) {
                    fontlib_SetCursorPosition(offsetX, fontlib_GetCursorY());
                }
                fontlib_ClearEOL();
                fontlib_SetCursorPosition(fontlib_GetCursorX(), i);
            }

            parse_draw_string(msg, true);

            // Draw stars if masking input, otherwise output plain text
            if (maskInput) {
                for (i = 1; i <= (int16_t)strlen(input); i++) {
                    parse_draw_string("*", true);
                }
            } else {
                parse_draw_string(input, true);
            }

            // Redraw cursor
            if (j < 500) {
                temp[0] = cursors[textIndex];
                parse_draw_string(temp, true);
            }

            gfx_BlitBuffer();

        }

        prevkey = key;
        
    } while (!done);

    /* When user hits enter, update screen output without cursor */

    // Loop backwards through wrapped lines and clear everything back to original X offset until redraw
    for (i = fontlib_GetCursorY(); i >= cursorY; i = i - 12) {
        fontlib_Home();
        if ((i == cursorY) && offsetX) {
            fontlib_SetCursorPosition(offsetX, fontlib_GetCursorY());
        }
        fontlib_ClearEOL();
        fontlib_SetCursorPosition(fontlib_GetCursorX(), i);
    }

    parse_draw_string(msg, true);

    // Draw stars if masking input, otherwise output plain text
    if (maskInput) {
        for (i = 1; i <= (int16_t)strlen(input); i++) {
            parse_draw_string("*", true);
        }
    } else {
        parse_draw_string(input, true);
    }

    gfx_BlitBuffer();
}

// Parse strings
void parse_draw_string(const char *string, const bool justDraw) {

    uint16_t i, j, x;
    uint8_t k, l, m, y;
    char temp[2];
    bool displayNextChar, fGcolorHasBeenSet, bGcolorHasBeenSet;

    // Empty input
    for (i = 0; i <= 256; i++) {
        parserData[i] = 0;
    }

    temp[1] = 0; // Put null terminator in temp

    displayNextChar = true;
    fGcolorHasBeenSet = false;
    bGcolorHasBeenSet = false;

    strcpy(parserData, string);

    // Begin displaying and parsing actual string
    for (i = 0; i < 256; i++) {
        
        if (!justDraw) {
            // Parse backslash-escaped characters
            if (parserData[i] == '\\') {

                // Parse \\, \", and \' by not displaing the backslash
                if ((parserData[i + 1] == '\'') || (parserData[i + 1] == '"') || (parserData[i + 1] == '\\')) {
                    i++;
                }

                // Parse \b
                if (parserData[i + 1] == 'b') {
                    
                    parserData[i - 1] = 0; // Delete backspaced character

                    // Shift string 3 chars to the left to remove backspaced character and \b
                    memmove(&parserData[i - 1], &parserData[i + 2], strlen(&parserData[i + 2]) + 1);

                    // Reset cursor position, print a space, and reset again
                    if (fontlib_GetCursorX() == 2) {
                        fontlib_SetCursorPosition(308, fontlib_GetCursorY() - 12);
                    } else {
                        fontlib_SetCursorPosition(fontlib_GetCursorX() - 6, fontlib_GetCursorY());
                    }
                    draw_str_update_buf(" ");
                    if (fontlib_GetCursorX() == 2) {
                        fontlib_SetCursorPosition(308, fontlib_GetCursorY() - 12);
                    } else {
                        fontlib_SetCursorPosition(fontlib_GetCursorX() - 6, fontlib_GetCursorY());
                    }

                    displayNextChar = false;
                    i -= 2; // Move i back as if deleted character never existed
                }
                
                // Parse \n
                if (parserData[i + 1] == 'n') {
                    draw_newline();
                    displayNextChar = false;
                    i++;
                }
                
                // Parse \r
                if (parserData[i + 1] == 'r') {
                    fontlib_SetCursorPosition(2, fontlib_GetCursorY());
                    displayNextChar = false;
                    i++;
                }
                
                // Parse \t
                if (parserData[i + 1] == 't') {
                    draw_str_update_buf("    ");
                    if (underlineText) {
                        gfx_SetColor(fontlib_GetForegroundColor());
                        gfx_HorizLine(fontlib_GetCursorX() - 6, fontlib_GetCursorY() + 10, 6);
                    }
                    displayNextChar = false;
                    i++;
                }
                
                // Parse \𝘯𝘯𝘯
                if ((parserData[i + 1] > 47) && (parserData[i + 1] < 56)) {
                    
                    // Increment m until the next character is not an octal digit or the sequence length is 3 digits
                    for (m = i + 1; (parserData[m] > 47) && (parserData[m] < 56) && (m < i + 4); m++);
                    
                    // m now equals the length of the sequence
                    m -= i + 1;
                    
                    // Replace last character with new character
                    parserData[i + m] = str_to_num(&parserData[i + 1], m, 8);
           
                    i += m;
                }
                
                // Parse \x𝘩𝘩
                if (parserData[i + 1] == 'x') {
                    
                    // Increment m until the next character is not a hex digit or the sequence length is 2 digits
                    for (m = i + 2; (((parserData[m] > 47) && (parserData[m] < 58)) || ((parserData[m] > 64) && (parserData[m] < 71)) || ((parserData[m] > 96) && (parserData[m] < 103))) && (m < i + 4); m++);
                    
                    // m now equals the length of the sequence
                    m -= i + 2;
                    
                    // Replace last character with new character
                    parserData[i + m + 1] = str_to_num(&parserData[i + 2], m, 16);
                    
                    i += m + 1;
                }

                // Parse \e[
                if ((parserData[i + 1] == 'e') && (parserData[i + 2] == '[')) {

                    /* Loop through remaining characters until a letter is encountered, then break
                       This leaves j as the index in the array of the first letter in the escape code
                       As the letter in an ANSI escape code is always last, this is the end of the code */
                    for (j = i + 3; j < 256; j++) {
                        if (((parserData[j] >= 65) && (parserData[j] <= 90)) || ((parserData[j] >= 97) && (parserData[j] <= 122))) break;
                    }

                    // Move cursor up
                    if (parserData[j] == 'A') {
                        fontlib_SetCursorPosition(fontlib_GetCursorX(), fontlib_GetCursorY() - (str_to_num(&parserData[i + 3], j - (i + 3), 10) * 12));
                    }

                    // Move cursor down
                    if (parserData[j] == 'B') {
                        fontlib_SetCursorPosition(fontlib_GetCursorX(), fontlib_GetCursorY() + (str_to_num(&parserData[i + 3], j - (i + 3), 10) * 12));
                    }

                    // Move cursor forward
                    if (parserData[j] == 'C') {
                        fontlib_SetCursorPosition(fontlib_GetCursorX() + (str_to_num(&parserData[i + 3], j - (i + 3), 10) * 6), fontlib_GetCursorY());
                    }

                    // Move cursor backward
                    if (parserData[j] == 'D') {
                        fontlib_SetCursorPosition(fontlib_GetCursorX() - (str_to_num(&parserData[i + 3], j - (i + 3), 10) * 6), fontlib_GetCursorY());
                    }

                    // Directly set cursor position
                    if ((parserData[j] == 'f') || (parserData[j] == 'H')) {
                        
                        // Move cursor to top-left (0,0)
                        if (parserData[j - 1] == '[') {
                            x = y = 0;
                        }
                        
                        // Store index of ';' in m
                        for (m = i + 3; m < j; m++) {
                            if (parserData[m] == ';')
                                break;
                        }
                        
                        // Retrieve & set cursor position
                        x = str_to_num(&parserData[m + 1], j - (m + 1), 10);
                        y = str_to_num(&parserData[i + 3], m - (i + 3), 10);
                        
                        fontlib_SetCursorPosition((x * 6) + 2, (y * 12) + 2);
                    }

                    // Clear screen
                    if ((parserData[j] == 'J') && (parserData[j - 1] == '2')) {
                        fontlib_ClearWindow();
                        for (x = 0; x < 52; x++) {
                            for (y = 0; y < 19; y++) {
                                scrBuffer[x][y].character = ' ';
                                scrBuffer[x][y].bold = false;
                                scrBuffer[x][y].italic = false;
                                scrBuffer[x][y].underline = false;
                                scrBuffer[x][y].fg_col = 7;
                                scrBuffer[x][y].bg_col = 0;
                            }
                        }
                    }

                    // Clear current line
                    if (parserData[j] == 'K') {
                        x = fontlib_GetCursorX();
                        y = fontlib_GetCursorY();
                        fontlib_ClearEOL();
                        for (k = (x - 2) / 6; k < 52; k++) {
                            scrBuffer[k][y].character = ' ';
                            scrBuffer[k][y].bold = false;
                            scrBuffer[k][y].italic = false;
                            scrBuffer[k][y].underline = false;
                            scrBuffer[k][y].fg_col = 7;
                            scrBuffer[k][y].bg_col = 0;
                        }
                    }

                    // Save current cursor location
                    if (parserData[j] == 's') {
                        cursorPos[0] = fontlib_GetCursorX();
                        cursorPos[1] = fontlib_GetCursorY();
                    }

                    // Load saved cursor location
                    if (parserData[j] == 'u') {
                        fontlib_SetCursorPosition(cursorPos[0], cursorPos[1]);
                    }

                    l = j + 2;

                    do {
                        // Seek to beginning of sequence
                        l = l - 2;
                        while (parserData[l - 1] == ';') {
                            l--;
                        }

                        // Handle text styling
                        if (parserData[j] == 'm') {

                            // Reset text attributes
                            if (((parserData[l - 1] == '[') && (l == j)) || ((parserData[l - 1] == '0') && ((parserData[l - 2] == ';') || (parserData[l - 2] == '[')))) {
                                fontlib_SetColors(0x07, 0x00);
                                fontlib_SetFont(terminus_font, 0);
                                underlineText = false;
                                italicText = false;
                                boldText = false;
                            }

                            // Bold text
                            if ((parserData[l - 1] == '1') && ((parserData[l - 2] == ';') || (parserData[l - 2] == '['))) {
                                if (italicText) {
                                    fontlib_SetFont(terminus_font_bold_italic, 0);
                                } else {
                                    fontlib_SetFont(terminus_font_bold, 0);
                                }

                                // Bright colors
                                if (fGcolorHasBeenSet) {
                                    if (fontlib_GetForegroundColor() < 8)
                                        fontlib_SetForegroundColor(fontlib_GetForegroundColor() + 8);
                                }
                                if (bGcolorHasBeenSet) {
                                    if (fontlib_GetBackgroundColor() < 8)
                                        fontlib_SetBackgroundColor(fontlib_GetBackgroundColor() + 8);
                                }
                                boldText = true;
                            }

                            fGcolorHasBeenSet = false;
                            bGcolorHasBeenSet = false;

                            // Italic text
                            if ((parserData[l - 1] == '3') && ((parserData[l - 2] == ';') || (parserData[l - 2] == '['))) {
                                if (boldText) {
                                    fontlib_SetFont(terminus_font_bold_italic, 0);
                                } else {
                                    fontlib_SetFont(terminus_font_italic, 0);
                                }
                                italicText = true;
                            }

                            // Underlined text
                            if ((parserData[l - 1] == '4') && ((parserData[l - 2] == ';') || (parserData[l - 2] == '['))) {
                                underlineText = true;
                            }

                            // Reverse video
                            if ((parserData[l - 1] == '7') && ((parserData[l - 2] == ';') || (parserData[l - 2] == '['))) {
                                k = fontlib_GetForegroundColor();
                                fontlib_SetForegroundColor(fontlib_GetBackgroundColor());
                                fontlib_SetBackgroundColor(k);
                            }

                            // Concealed text
                            if ((parserData[l - 1] == '8') && ((parserData[l - 2] == ';') || (parserData[l - 2] == '['))) {
                                fontlib_SetForegroundColor(fontlib_GetBackgroundColor());
                            }

                            // Colored foreground text
                            if ((parserData[l - 2] == '3') && (parserData[l - 1] == '0')) {
                                fontlib_SetForegroundColor(0x00);
                                fGcolorHasBeenSet = true;
                            }
                            if ((parserData[l - 2] == '3') && (parserData[l - 1] == '1')) {
                                fontlib_SetForegroundColor(0x01);
                                fGcolorHasBeenSet = true;
                            }
                            if ((parserData[l - 2] == '3') && (parserData[l - 1] == '2')) {
                                fontlib_SetForegroundColor(0x02);
                                fGcolorHasBeenSet = true;
                            }
                            if ((parserData[l - 2] == '3') && (parserData[l - 1] == '3')) {
                                fontlib_SetForegroundColor(0x03);
                                fGcolorHasBeenSet = true;
                            }
                            if ((parserData[l - 2] == '3') && (parserData[l - 1] == '4')) {
                                fontlib_SetForegroundColor(0x04);
                                fGcolorHasBeenSet = true;
                            }
                            if ((parserData[l - 2] == '3') && (parserData[l - 1] == '5')) {
                                fontlib_SetForegroundColor(0x05);
                                fGcolorHasBeenSet = true;
                            }
                            if ((parserData[l - 2] == '3') && (parserData[l - 1] == '6')) {
                                fontlib_SetForegroundColor(0x06);
                                fGcolorHasBeenSet = true;
                            }
                            if ((parserData[l - 2] == '3') && (parserData[l - 1] == '7')) {
                                fontlib_SetForegroundColor(0x07);
                                fGcolorHasBeenSet = true;
                            }

                            // Colored background text
                            if ((parserData[l - 2] == '4') && (parserData[l - 1] == '0')) {
                                fontlib_SetBackgroundColor(0x00);
                                bGcolorHasBeenSet = true;
                            }
                            if ((parserData[l - 2] == '4') && (parserData[l - 1] == '1')) {
                                fontlib_SetBackgroundColor(0x01);
                                bGcolorHasBeenSet = true;
                            }
                            if ((parserData[l - 2] == '4') && (parserData[l - 1] == '2')) {
                                fontlib_SetBackgroundColor(0x02);
                                bGcolorHasBeenSet = true;
                            }
                            if ((parserData[l - 2] == '4') && (parserData[l - 1] == '3')) {
                                fontlib_SetBackgroundColor(0x03);
                                bGcolorHasBeenSet = true;
                            }
                            if ((parserData[l - 2] == '4') && (parserData[l - 1] == '4')) {
                                fontlib_SetBackgroundColor(0x04);
                                bGcolorHasBeenSet = true;
                            }
                            if ((parserData[l - 2] == '4') && (parserData[l - 1] == '5')) {
                                fontlib_SetBackgroundColor(0x05);
                                bGcolorHasBeenSet = true;
                            }
                            if ((parserData[l - 2] == '4') && (parserData[l - 1] == '6')) {
                                fontlib_SetBackgroundColor(0x06);
                                bGcolorHasBeenSet = true;
                            }
                            if ((parserData[l - 2] == '4') && (parserData[l - 1] == '7')) {
                                fontlib_SetBackgroundColor(0x07);
                                bGcolorHasBeenSet = true;
                            }
                        }
                    } while (l > i + 2);

                    displayNextChar = false;
                    i = j;
                }
            }
        }
        if (parserData[i] == 0) break;

        // Output next character
        if (displayNextChar) {
            temp[0] = parserData[i];
            draw_str_update_buf(temp);
            if (underlineText) {
                gfx_SetColor(fontlib_GetForegroundColor());
                gfx_HorizLine(fontlib_GetCursorX() - 6, fontlib_GetCursorY() + 10, 6);
            }
        } else {
            displayNextChar = true;
        }
    }
}

// Draw a string and update the screen buffer
void draw_str_update_buf(const char *string) {
    
    uint16_t i, j;
    uint8_t x, y;
    char temp[2];
    
    temp[1] = 0; // Put null terminator in temp
    
    // Loop through input
    for (i = 0; i < 256; i++) {
        if (string[i] == 0) break;
        
        // Get x & y coords
        x = (fontlib_GetCursorX() - 2) / 6;
        y = (fontlib_GetCursorY() - 2) / 12;
        
        temp[0] = string[i];
        fontlib_DrawString(temp);
        
        // If the screen scrolled...
        if ((((fontlib_GetCursorX() - 2) / 6) == 1) && (((fontlib_GetCursorY() - 2) / 12) == 18) && (y == 18)) {
            // Scroll the contents of the screen buffer
            memmove(&scrBuffer[0][0], &scrBuffer[0][1], 936 * sizeof(char_styled_t));
            for (j = 0; j < 52; j++) {
                scrBuffer[j][18].character = ' ';
                scrBuffer[j][18].bold = false;
                scrBuffer[j][18].italic = false;
                scrBuffer[j][18].underline = false;
                scrBuffer[j][18].fg_col = 7;
                scrBuffer[j][18].bg_col = 0;
            }
            
            // Erase the bottom line
            gfx_SetColor(0);
            gfx_FillRectangle(0, 218, 320, 22);
            
            fontlib_SetCursorPosition(2, fontlib_GetCursorY());
            
            x = (fontlib_GetCursorX() - 2) / 6;
            y = (fontlib_GetCursorY() - 2) / 12;
            
            fontlib_DrawString(temp);
        }
        
        scrBuffer[x][y].character = string[i];
        scrBuffer[x][y].bold = boldText;
        scrBuffer[x][y].italic = italicText;
        scrBuffer[x][y].underline = underlineText;
        scrBuffer[x][y].fg_col = fontlib_GetForegroundColor();
        scrBuffer[x][y].bg_col = fontlib_GetBackgroundColor();
    }
}

// Draw an integer and update the screen buffer
void draw_int_update_buf(int number, const uint8_t length) {

    char str[length + 1];
    uint8_t i, len;
    
    snprintf(str, length + 1, "%d", number); // Convert integer to string
    
    // Pad with 0s if necessary
    len = strlen(str);
    if (len < length) {
        memmove(&str[length - len], &str[0], len + 1);
        for (i = 0; i < len; i++) {
            str[i] = '0';
        }
    }
    
    draw_str_update_buf(str);
}

// Draw a newline, accounting for scrolling
void draw_newline(void) {
    
    uint8_t j, y = fontlib_GetCursorY();
    
    if (((y - 2) / 12) == 18) {
        fontlib_ScrollWindowDown(); // Manually scroll window
        
        // Scroll the contents of the screen buffer
        memmove(&scrBuffer[0][0], &scrBuffer[0][1], 936 * sizeof(char_styled_t));
        for (j = 0; j < 52; j++) {
            scrBuffer[j][18].character = ' ';
            scrBuffer[j][18].bold = false;
            scrBuffer[j][18].italic = false;
            scrBuffer[j][18].underline = false;
            scrBuffer[j][18].fg_col = 7;
            scrBuffer[j][18].bg_col = 0;
        }
        
        // Erase the bottom line
        gfx_SetColor(0);
        gfx_FillRectangle(0, 218, 320, 22);
        
        fontlib_SetCursorPosition(2, y);
    } else {
        fontlib_Newline();
    }
}

// Convert any base (up to base 36) string literal to 8-bit decimal integer
uint8_t str_to_num(const char *string, const uint8_t length, const uint8_t base) {

    uint8_t returnValue = 0, i = 0, j = 0;

    // Loop through string, convert each ASCII character to it's decimal form, and multiply it times the current place value
    for (i = length; i > 0; i--) {
        returnValue += (string[i - 1] - (((string[i - 1] > 47) && (string[i - 1] < 58)) * 48) - (((string[i - 1] > 64) && (string[i - 1] < 91)) * 55) - (((string[i - 1] > 96) && (string[i - 1] < 123)) * 87)) * pow(base, j);
        j++;
    }

    return returnValue;
}


// Saves the shell state and runs a program
int run_prgm(char *prgm, char *args) {
    
    int ret;
    
    // Make sure program actually exists
    settingsAppvar = ti_OpenVar(prgm, "r", TI_PRGM_TYPE);
    if (settingsAppvar == 0) {
        return -1;
    }
    ti_Close(settingsAppvar);
    
    // Set save state variables
    sdRetFromPrgm = true;
    sdCursorPos[0] = fontlib_GetCursorX();
    sdCursorPos[1] = fontlib_GetCursorY();
    sdTextColors[0] = fontlib_GetForegroundColor();
    sdTextColors[1] = fontlib_GetBackgroundColor();
    
    // Save shell state
    settingsAppvar = ti_Open("CEshSett", "r+");
    
    ti_Seek(sizeof(char) * 66, SEEK_SET, settingsAppvar);
    ti_Write(&sdRetFromPrgm, sizeof(bool), 1, settingsAppvar);
    ti_Write(&sdCursorPos, sizeof(uint16_t), 2, settingsAppvar);
    ti_Write(&sdTextColors, sizeof(uint8_t), 2, settingsAppvar);
    
    ti_SetArchiveStatus(true, settingsAppvar);
    ti_Close(settingsAppvar);
    
    // Save screen state
    settingsAppvar = ti_Open("CEshScrA", "w+");
        
    gfx_GetSprite(sdScr, 0, 0);
    ti_Write(&sdScr, sizeof(char), 20162, settingsAppvar);
    
    gfx_GetSprite(sdScr, 160, 0);
    ti_Write(&sdScr, sizeof(char), 20162, settingsAppvar);
    
    //ti_SetArchiveStatus(true, settingsAppvar);
    ti_Close(settingsAppvar);
    
    settingsAppvar = ti_Open("CEshScrB", "w+");
        
    gfx_GetSprite(sdScr, 0, 120);
    ti_Write(&sdScr, sizeof(char), 20162, settingsAppvar);
    
    gfx_GetSprite(sdScr, 160, 120);
    ti_Write(&sdScr, sizeof(char), 20162, settingsAppvar);
    
    //ti_SetArchiveStatus(true, settingsAppvar);
    ti_Close(settingsAppvar);
    
    gfx_End();
    
    ret = os_RunPrgm(prgm, NULL, 0, (os_runprgm_callback_t)main);
    cesh_Init();
    
    return ret;
}