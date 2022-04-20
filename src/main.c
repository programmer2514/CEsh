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
#include "fonts/fonts.h"

/* Macro definitions */
#define BLACK  0x00
#define RED    0x01
#define GREEN  0x02
#define YELLOW 0x03
#define BLUE   0x04
#define PURPLE 0x05
#define CYAN   0x06
#define WHITE  0x07

#define BRIGHT_BLACK  0x08
#define BRIGHT_RED    0x09
#define BRIGHT_GREEN  0x0A
#define BRIGHT_YELLOW 0x0B
#define BRIGHT_BLUE   0x0C
#define BRIGHT_PURPLE 0x0D
#define BRIGHT_CYAN   0x0E
#define BRIGHT_WHITE  0x0F

#define SCR_WIDTH   52
#define SCR_HEIGHT  19
#define FONT_WIDTH  6
#define FONT_HEIGHT 12

#define SCR_WIDTH_P  316
#define SCR_HEIGHT_P 236
#define SCR_OFFSET_X 2
#define SCR_OFFSET_Y 2

#define INPUT_LENGTH    257
#define PATH_LENGTH     257
#define USER_PWD_LENGTH 33

#define BUFFER_SIZE (SCR_WIDTH * SCR_HEIGHT)

#define getDayOfWeek(yr,mon,day) (((day) += (mon) < 3 ? (yr)-- : (yr) - 2, 23*(mon)/9 + (day) + 4 + (yr)/4 - (yr)/100 + (yr)/400)%7)
#define inRange(input,min,max) ((input)>=(min)&&(input)<=(max))

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
void cesh_Splash(void);
void cesh_Shell(void);
void cesh_PreGC(void);
void cesh_End(void);
void parse_user_input(void);
void get_user_input(const char *msg, const bool maskInput, const bool disableRecall, const uint16_t offsetX);
void parse_draw_string(const char *string);
void draw_str_update_buf(const char *string);
void draw_int_update_buf(int number, const uint8_t length);
void draw_newline(void);
void print_spaces(uint16_t x, uint16_t y, uint16_t num);
uint16_t str_to_num(const char *string, const uint8_t length, const uint8_t base);
uint8_t get_single_key_pressed(void);
int run_prgm(char *prgm, char *args);


/* Variable declarations */
const char *P_CALC_NAME = (char *)0x3B000E;
const char CURSOR_INDEX[4] = {129, 130, 128, 95};
const char DOW_NAMES[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char MON_NAMES[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char KEY_MAP[4][57] = {{0,0,0,0,0,0,0,0,0,0,'+' ,'-','*','/','^',0,0,'_','3','6','9',')',0  ,0,0,'.','2','5','8','(',0  ,0  ,0,'0','1','4','7',',',0  ,0  ,0,0,0  ,0  ,0  ,0  ,'\\',0  ,0,0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0,0,0,0   ,']','[','&','|',0,0,0  ,0  ,0  ,0  ,'}',0  ,0,0,'!',0  ,0  ,0  ,'{',0  ,0  ,0,0  ,0  ,0  ,0  ,'=',0  ,0  ,0,0,0  ,0  ,0  ,0  ,0   ,0  ,0,0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0,0,0,'\'','W','R','M','H',0,0,'?','@','V','Q','L','G',0,0,':','Z','U','P','K','F','C',0,' ','Y','T','O','J','E','B',0,0,'X','S','N','I','D' ,'A',0,0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0,0,0,'"' ,'w','r','m','h',0,0,0  ,0  ,'v','q','l','g',0,0,';','z','u','p','k','f','c',0,' ','y','t','o','j','e','b',0,0,'x','s','n','i','d' ,'a',0,0,0,0,0,0,0,0,0}};

char input[INPUT_LENGTH];
char parserData[INPUT_LENGTH];
char path[PATH_LENGTH];
char calcName[25];
uint8_t textIndex = 0;

uint16_t cursorPos[2] = {SCR_OFFSET_X, SCR_OFFSET_Y};
bool startOnNewLine = true;
bool underlineText = false;
bool italicText = false;
bool boldText = false;

char user[USER_PWD_LENGTH];
char pwd[USER_PWD_LENGTH];
bool sdRetFromPrgm = false;
uint16_t sdCursorPos[2] = {SCR_OFFSET_X, SCR_OFFSET_Y};
uint8_t sdTextColors[2] = {WHITE, BLACK};
bool sdUnderlineText = false;
bool sdItalicText = false;
bool sdBoldText = false;

ti_var_t settingsAppvar;
bool settingsAppvarExists = true;

uint16_t dateTime[7];
bool lastLoginHappened = false;

char_styled_t scrBuffer[SCR_HEIGHT][SCR_WIDTH];
bool shouldScroll;


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
    settingsAppvar = ti_Open("CEshSett", "r");

    if (settingsAppvar != 0) {
        ti_Read(&user, sizeof(char), USER_PWD_LENGTH, settingsAppvar);
        ti_Read(&pwd, sizeof(char), USER_PWD_LENGTH, settingsAppvar);
        ti_Read(&sdRetFromPrgm, sizeof(bool), 1, settingsAppvar);
    } else {
        settingsAppvarExists = false;
        settingsAppvar = ti_Open("CEshSett", "w+");
        ti_Write(&user, sizeof(char), USER_PWD_LENGTH, settingsAppvar);
        ti_Write(&pwd, sizeof(char), USER_PWD_LENGTH, settingsAppvar);
        ti_Write(&sdRetFromPrgm, sizeof(bool), 1, settingsAppvar);
        ti_Write(&sdCursorPos, sizeof(uint16_t), 2, settingsAppvar);
        ti_Write(&sdTextColors, sizeof(uint8_t), 2, settingsAppvar);
        ti_Write(&dateTime, sizeof(uint16_t), 7, settingsAppvar);
        ti_Write(&sdUnderlineText, sizeof(bool), 1, settingsAppvar);
        ti_Write(&sdItalicText, sizeof(bool), 1, settingsAppvar);
        ti_Write(&sdBoldText, sizeof(bool), 1, settingsAppvar);
        ti_SetArchiveStatus(true, settingsAppvar);
    }

    if (sdRetFromPrgm) {
        ti_Read(&sdCursorPos, sizeof(uint16_t), 2, settingsAppvar);
        ti_Read(&sdTextColors, sizeof(uint8_t), 2, settingsAppvar);
        ti_Seek(sizeof(uint16_t) * 7, SEEK_SET, settingsAppvar);
        ti_Read(&sdUnderlineText, sizeof(bool), 1, settingsAppvar);
        ti_Read(&sdItalicText, sizeof(bool), 1, settingsAppvar);
        ti_Read(&sdBoldText, sizeof(bool), 1, settingsAppvar);
    }

    ti_Close(settingsAppvar);

    // Attempt to load screen state
    settingsAppvar = ti_Open("CEshSBuf", "r");
    if (settingsAppvar != 0) {
        ti_Read(&scrBuffer, sizeof(char_styled_t), BUFFER_SIZE, settingsAppvar);
        ti_Close(settingsAppvar);
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
    gfx_SetTextBGColor(sdTextColors[1]);
    gfx_SetTextFGColor(sdTextColors[0]);
    gfx_SetColor(sdTextColors[0]);
    fontlib_SetColors(sdTextColors[0], sdTextColors[1]);

    fontlib_SetCursorPosition(sdCursorPos[0], sdCursorPos[1]);

    strcpy(path, "/");
    strcpy(calcName, P_CALC_NAME);
}

// First time setup
void cesh_Setup(void) {

    uint8_t i;
    char pwd_tmp[USER_PWD_LENGTH];
    
    cesh_Splash();

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
    settingsAppvar = ti_Open("CEshSett", "r+");
    ti_Write(&user, sizeof(char), USER_PWD_LENGTH, settingsAppvar);
    ti_Write(&pwd, sizeof(char), USER_PWD_LENGTH, settingsAppvar);
    ti_SetArchiveStatus(true, settingsAppvar);
    ti_Close(settingsAppvar);

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
    gfx_Sprite(imgSplash, (LCD_WIDTH - imgSplash_width) / 2, (LCD_HEIGHT - imgSplash_height) * 7 / 24);
    fontlib_SetCursorPosition((LCD_WIDTH - 29*FONT_WIDTH) / 2, (LCD_HEIGHT - FONT_HEIGHT) * 2 / 3);
    fontlib_DrawString("Initializing first-time setup");
    for (i = 0; i < (random() % 18) + 6; i++) {
        gfx_SetColor(inRange((i % 6),1,3) ? WHITE : BLACK);
        gfx_FillCircle((LCD_WIDTH / 2) - 18, LCD_HEIGHT * 3 / 4, 3);
        gfx_SetColor(inRange((i % 6),2,4) ? WHITE : BLACK);
        gfx_FillCircle(LCD_WIDTH / 2, LCD_HEIGHT * 3 / 4, 3);
        gfx_SetColor(inRange((i % 6),3,5) ? WHITE : BLACK);
        gfx_FillCircle((LCD_WIDTH / 2) + 18, LCD_HEIGHT * 3 / 4, 3);
        gfx_BlitBuffer();
        delay(800);
    }
    gfx_FillScreen(BLACK);
    fontlib_SetCursorPosition(SCR_OFFSET_X, SCR_OFFSET_Y);
}

// Main shell loop
void cesh_Shell(void) {

    uint8_t temp;
    uint8_t i;
    uint8_t day, mon, sec, min, hr;
    uint16_t yr;
    uint16_t dateTimeTemp[7];
    bool fts = false;
    
    if (!sdRetFromPrgm) {
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
            draw_str_update_buf(calcName);
            get_user_input(" login: ", false, true, (strlen(calcName) * FONT_WIDTH) + SCR_OFFSET_X);
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

        settingsAppvar = ti_Open("CEshSett", "r+");
        ti_Seek((2 * USER_PWD_LENGTH) + 7, SEEK_SET, settingsAppvar); // Go to correct location in settings appvar

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
        draw_str_update_buf(calcName);
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
    settingsAppvar = ti_Open("CEshSBuf", "w+");
    ti_Write(&scrBuffer, sizeof(char_styled_t), BUFFER_SIZE, settingsAppvar);
    ti_Close(settingsAppvar);

    gfx_End();
}

// End program
void cesh_End(void) {
    gfx_End();

    // Mark shell as exited
    sdRetFromPrgm = false;
    settingsAppvar = ti_Open("CEshSett", "r+");
    ti_Seek(2 * USER_PWD_LENGTH, SEEK_SET, settingsAppvar);
    ti_Write(&sdRetFromPrgm, sizeof(bool), 1, settingsAppvar);
    ti_SetArchiveStatus(true, settingsAppvar);
    ti_Close(settingsAppvar);
    settingsAppvar = ti_Open("CEshHist", "r");
    ti_SetArchiveStatus(true, settingsAppvar);
    ti_Close(settingsAppvar);

    exit(0);
}

// Parse user input
void parse_user_input(void) {

    uint16_t i, j, k;
    char *ptr;
    int retval, size;
    uint8_t numargs = 1;
    uint8_t *arglocs = malloc(1);
    bool inQuotes = false;
    
    settingsAppvar = ti_Open("CEshHist", "r+");
    if (!settingsAppvar) 
        settingsAppvar = ti_Open("CEshHist", "w+");
    size = ti_GetSize(settingsAppvar);
    
    // If it has fewer than 250 entries, seek to the end of the file
    if (size < (250 * INPUT_LENGTH)) {
        ti_Seek(0, SEEK_END, settingsAppvar);
        
    // Otherwise, delete the oldest entry
    } else { 
        ptr = ti_GetDataPtr(settingsAppvar);
        memmove(ptr, ptr + INPUT_LENGTH, size - INPUT_LENGTH);
        ti_Seek(size - INPUT_LENGTH, SEEK_SET, settingsAppvar);
    }
    
    // Add command to history buffer & close appvar
    ti_Write(&input, sizeof(char), INPUT_LENGTH, settingsAppvar);
    ti_Close(settingsAppvar);

    arglocs[0] = 0; // Pre-fill location of command name, as it's always 0

    // Pre-search input to define argument delimiters and remove quotes
    for (i = 0; i < INPUT_LENGTH; i++) {
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
        
        j = 1; // 1 until args are parsed, then 0
        k = 1; // 1 if parsing, 0 if not
        
        for (i = 1; i < numargs; i++) {
            if (j) {
                if (!strcmp(&input[arglocs[i]], "-n")) {
                    startOnNewLine = false;
                } else if (!strcmp(&input[arglocs[i]], "-e")) {
                    k = 1;
                } else if (!strcmp(&input[arglocs[i]], "-E")) {
                    k = 2;
                } else {
                    j = 0;
                }
            }
            if (!j) {
                if (k == 1)
                    parse_draw_string(&input[arglocs[i]]);
                if (k == 2)
                    draw_str_update_buf(&input[arglocs[i]]);
                if (i < numargs - 1) {
                    draw_str_update_buf(" ");
                }
            }
        }

    // Command: history
    } else if (!strcmp(input, "history")) {

        startOnNewLine = false;
        draw_newline();
        
        settingsAppvar = ti_Open("CEshHist", "r");
        size = ti_GetSize(settingsAppvar);
        
        if (numargs > 1) {
            if (!strcmp(&input[arglocs[1]], "-c")) {
                ti_Delete("CEshHist");
            } else if ((numargs > 2) && (!strcmp(&input[arglocs[1]], "-d"))) {
                ptr = ti_GetDataPtr(settingsAppvar);
                j = str_to_num(&input[arglocs[2]], strlen(&input[arglocs[2]]), 10);
                memmove(ptr + INPUT_LENGTH, ptr, (j - 1) * INPUT_LENGTH);
                ti_Resize(size - INPUT_LENGTH, settingsAppvar);
            } else {
                j = str_to_num(&input[arglocs[1]], strlen(&input[arglocs[1]]), 10);
                for (i = (size / INPUT_LENGTH) - j; i < (size / INPUT_LENGTH); i++) {
                    ti_Seek(i * INPUT_LENGTH, SEEK_SET, settingsAppvar);
                    ti_Read(&input, sizeof(char), INPUT_LENGTH, settingsAppvar);
                    
                    draw_int_update_buf(i + 1, 1 + ((i + 1) >= 10) + ((i + 1) >= 100));
                    draw_str_update_buf("  ");
                    draw_str_update_buf(input);
                    parse_draw_string("\\n");
                }
            }
        } else {
            for (i = 0; i < (size / INPUT_LENGTH); i++) {
                ti_Seek(i * INPUT_LENGTH, SEEK_SET, settingsAppvar);
                ti_Read(&input, sizeof(char), INPUT_LENGTH, settingsAppvar);
                
                draw_int_update_buf(i + 1, 1 + ((i + 1) >= 10) + ((i + 1) >= 100));
                draw_str_update_buf("  ");
                draw_str_update_buf(input);
                parse_draw_string("\\n");
            }
        }
        
        ti_Close(settingsAppvar);

    // Debug command (remove in release)
    } else if (!strcmp(input, "dbg")) {

        startOnNewLine = true;
        draw_newline();
        for (i = 0; i < numargs; i++) {
            draw_int_update_buf(i, 1);
            draw_str_update_buf(": ");
            draw_str_update_buf(&input[arglocs[i]]);
            parse_draw_string("\\n");
        }

    // Prevent shell running itself
    } else if (!strcmp(input, "./CESH")) {

        draw_newline();
        draw_str_update_buf("No");

    // Command: .
    } else if (input[0] == '.') {

        if (input[1] == '/') {
            retval = run_prgm(&input[2], "null");

            draw_newline();
            draw_str_update_buf("Error ");
            draw_int_update_buf(retval, 2);
            draw_str_update_buf(": ");
            if (retval == -1) {
                draw_str_update_buf(input);
                draw_str_update_buf(": No such file or directory");
            } else if (retval == -2) {
                draw_str_update_buf(input);
                draw_str_update_buf(": Not enough memory");
            } else {
                draw_str_update_buf(input);
                draw_str_update_buf(": Unspecified error");
            }
        } else {
            draw_str_update_buf(".: usage: ./filename [arguments]");
        }

    // If not a built-in command and not empty
    } else if (strcmp(input, "") && strcmp(input, "exit") && (input[0] != ' ') && (input[0] != 0)) {

        draw_newline();
        draw_str_update_buf(input);
        draw_str_update_buf(": command not found");

    }

    gfx_BlitBuffer();
}

// Get user input
void get_user_input(const char *msg, const bool maskInput, const bool disableRecall, const uint16_t offsetX) {

    bool done = false;
    uint16_t i, y,
             cursorOffset = 0,
             cursorY = 0;
    int16_t j = 0,
            k,
            lineWrap;
    uint8_t key,
            histOffset = 1;
    char temp[2] = {0, 0};

    // Empty output
    for (i = 0; i <= INPUT_LENGTH; i++) {
        input[i] = 0;
    }

    textIndex = 3; // Reset cursor state
    lineWrap = offsetX + FONT_WIDTH; // Initialize lineWrap with current X offset + cursor width
    cursorY = fontlib_GetCursorY();

    gfx_BlitBuffer();

    do {
        // Update keyboard data
        key = get_single_key_pressed();

        // Blink Cursor
        if (j == 0) { // Update screen output w/ cursor

            print_spaces(offsetX, cursorY, strlen(msg) + strlen(input) + 1);
            draw_str_update_buf(msg);

            // Draw stars if masking input, otherwise output plain text
            if (maskInput) {
                for (i = 1; i <= strlen(input) + 1; i++) {
                    if (i == strlen(input) - cursorOffset + 1) {
                        temp[0] = CURSOR_INDEX[textIndex];
                        fontlib_DrawString(temp); // Draw cursor
                    } else if (i <= strlen(input)) {
                        draw_str_update_buf("*");
                    }
                }
            } else {
                i = strlen(input) - cursorOffset;
                temp[0] = input[i];
                input[i] = CURSOR_INDEX[textIndex];
                if (!cursorOffset)
                    input[i + 1] = 0;
                draw_str_update_buf(input);
                input[i] = temp[0];
            }

            gfx_BlitBuffer();

        } else if (j == 500) { // Update screen output w/o cursor

            print_spaces(offsetX, cursorY, strlen(msg) + strlen(input) + 1);
            draw_str_update_buf(msg);

            // Draw stars if masking input, otherwise output plain text
            if (maskInput) {
                for (i = 1; i <= (int16_t)strlen(input); i++) {
                    draw_str_update_buf("*");
                }
            } else {
                draw_str_update_buf(input);
            }

            gfx_BlitBuffer();

        } else if (j == 1000) { // Reset j once cursor has flashed one time
            j = -1;
        }

        j++;

        // Update output
        if (key) {
            
            if (KEY_MAP[textIndex][key]) {
                if (strlen(input) < (INPUT_LENGTH - 2)) {
                    if (cursorOffset) {
                        memmove(&input[strlen(input) - cursorOffset], &input[strlen(input) - cursorOffset - 1], strlen(input) - (strlen(input) - cursorOffset - 1));
                        input[strlen(input) - cursorOffset - 1] = KEY_MAP[textIndex][key];
                    } else {
                        input[strlen(input)] = KEY_MAP[textIndex][key];
                    }
                    input[strlen(input) + 1] = 0;
                    lineWrap += FONT_WIDTH;
                }
            } else {
                switch (key) {
                    case 1: // Down
                        if (!disableRecall) {
                            print_spaces(offsetX, cursorY, strlen(msg) + strlen(input) + 1);
                            // Seek to correct history offset
                            histOffset = (histOffset > 2) ? histOffset - 2 : 0;
                            settingsAppvar = ti_Open("CEshHist", "r");
                            if (settingsAppvar && histOffset) {
                                ti_Seek(0 - (histOffset * INPUT_LENGTH), SEEK_END, settingsAppvar);
                                ti_Read(&input, sizeof(char), INPUT_LENGTH, settingsAppvar);
                            }
                            ti_Close(settingsAppvar);
                            y = cursorY;
                            lineWrap = offsetX + ((strlen(input) + 1) * FONT_WIDTH);
                            while (lineWrap >= (((SCR_WIDTH - 1) * FONT_WIDTH) + SCR_OFFSET_X)) {
                                if (y >= (((SCR_HEIGHT - 1) * FONT_HEIGHT) + SCR_OFFSET_Y)) {
                                    cursorY -= FONT_HEIGHT;
                                    fontlib_ScrollWindowDown(); // Manually scroll window

                                    // Scroll the contents of the screen buffer
                                    memmove(&scrBuffer[0][0], &scrBuffer[1][0], (BUFFER_SIZE - SCR_WIDTH) * sizeof(char_styled_t));
                                    for (k = 0; k < SCR_WIDTH; k++) {
                                        scrBuffer[SCR_HEIGHT - 1][k].character = ' ';
                                        scrBuffer[SCR_HEIGHT - 1][k].bold = false;
                                        scrBuffer[SCR_HEIGHT - 1][k].italic = false;
                                        scrBuffer[SCR_HEIGHT - 1][k].underline = false;
                                        scrBuffer[SCR_HEIGHT - 1][k].fg_col = WHITE;
                                        scrBuffer[SCR_HEIGHT - 1][k].bg_col = BLACK;
                                    }

                                    // Erase the bottom line
                                    gfx_SetColor(BLACK);
                                    gfx_FillRectangle(SCR_OFFSET_X, (((SCR_HEIGHT - 1) * FONT_HEIGHT) + SCR_OFFSET_Y), SCR_WIDTH_P, FONT_HEIGHT);

                                    fontlib_SetCursorPosition(SCR_OFFSET_X, (((SCR_HEIGHT - 1) * FONT_HEIGHT) + SCR_OFFSET_Y));
                                }
                                lineWrap -= SCR_WIDTH * FONT_WIDTH;
                                y += FONT_HEIGHT;
                            }
                            if (!histOffset) {
                                for (i = 0; i <= 256; i++) {
                                    input[i] = 0;
                                }
                                lineWrap = offsetX + FONT_WIDTH;
                            }
                            histOffset++;
                            cursorOffset = 0;
                        }
                        break;
                    case 2: // Left
                        if (cursorOffset < strlen(input)) cursorOffset++;
                        j = -1;
                        break;
                    case 3: // Right
                        if (cursorOffset > 0) cursorOffset--;
                        j = -1;
                        break;
                    case 4: // Up
                        if (!disableRecall) {
                            print_spaces(offsetX, cursorY, strlen(msg) + strlen(input) + 1);
                            settingsAppvar = ti_Open("CEshHist", "r");
                            if (settingsAppvar) {
                                ti_Seek(0 - (histOffset * INPUT_LENGTH), SEEK_END, settingsAppvar);
                                ti_Read(&input, sizeof(char), INPUT_LENGTH, settingsAppvar);
                            }
                            histOffset = (histOffset + 1) * (histOffset <= (ti_GetSize(settingsAppvar) / INPUT_LENGTH));
                            ti_Close(settingsAppvar);
                            y = cursorY;
                            lineWrap = offsetX + ((strlen(input) + 1) * FONT_WIDTH);
                            while (lineWrap >= (((SCR_WIDTH - 1) * FONT_WIDTH) + SCR_OFFSET_X)) {
                                if (y >= (((SCR_HEIGHT - 1) * FONT_HEIGHT) + SCR_OFFSET_Y)) {
                                    cursorY -= FONT_HEIGHT;
                                    fontlib_ScrollWindowDown(); // Manually scroll window

                                    // Scroll the contents of the screen buffer
                                    memmove(&scrBuffer[0][0], &scrBuffer[1][0], (BUFFER_SIZE - SCR_WIDTH) * sizeof(char_styled_t));
                                    for (k = 0; k < SCR_WIDTH; k++) {
                                        scrBuffer[SCR_HEIGHT - 1][k].character = ' ';
                                        scrBuffer[SCR_HEIGHT - 1][k].bold = false;
                                        scrBuffer[SCR_HEIGHT - 1][k].italic = false;
                                        scrBuffer[SCR_HEIGHT - 1][k].underline = false;
                                        scrBuffer[SCR_HEIGHT - 1][k].fg_col = WHITE;
                                        scrBuffer[SCR_HEIGHT - 1][k].bg_col = BLACK;
                                    }

                                    // Erase the bottom line
                                    gfx_SetColor(BLACK);
                                    gfx_FillRectangle(SCR_OFFSET_X, (((SCR_HEIGHT - 1) * FONT_HEIGHT) + SCR_OFFSET_Y), SCR_WIDTH_P, FONT_HEIGHT);

                                    fontlib_SetCursorPosition(SCR_OFFSET_X, (((SCR_HEIGHT - 1) * FONT_HEIGHT) + SCR_OFFSET_Y));
                                }
                                lineWrap -= SCR_WIDTH * FONT_WIDTH;
                                y += FONT_HEIGHT;
                            }
                            cursorOffset = 0;
                        }
                        break;
                    case 9: // Enter
                        done = true;
                        break;
                    case 15: // Clear
                        // Clear text back
                        print_spaces(offsetX, cursorY, strlen(msg) + strlen(input) + 1);
                        for (i = 0; i <= 256; i++) {
                            input[i] = 0;
                        }
                        lineWrap = offsetX + FONT_WIDTH;
                        histOffset = 1;
                        cursorOffset = 0;
                        break;
                    case 32: // Stat
                        cursorOffset = 0;
                        j = -1;
                        break;
                    case 40: // X
                        cursorOffset = strlen(input);
                        j = -1;
                        break;
                    case 48: // Alpha
                        if (textIndex != 2) {
                            textIndex = 2;
                        } else {
                            textIndex = 3;
                        }
                        break;
                    case 54: // 2nd
                        if ((textIndex == 2) || (textIndex == 3)) {
                            textIndex = 0;
                        } else if (textIndex == 0) {
                            textIndex = 1;
                        } else if (textIndex == 1) {
                            textIndex = 3;
                        }
                        break;
                    case 55: // Mode
                        if (textIndex < 2) {
                            cesh_End();
                        }
                        break;
                    case 56: // Del
                        print_spaces(offsetX, cursorY, strlen(msg) + strlen(input) + 1);
                        if (strlen(input)) {
                            if (cursorOffset) {
                                memmove(&input[strlen(input) - cursorOffset], &input[strlen(input) - cursorOffset + 1], strlen(input) - (strlen(input) - cursorOffset + 1));
                                cursorOffset--;
                            }
                            input[strlen(input)-1] = 0;
                            lineWrap = lineWrap - 6;
                        }
                        break;
                    default:
                        break;
                }
            }

            /* Immediately update screen output when input string changed */

            // Update lineWrap and cursorY

            if (lineWrap >= (((SCR_WIDTH - 1) * FONT_WIDTH) + SCR_OFFSET_X)) {
                lineWrap = SCR_OFFSET_X - FONT_WIDTH;
                if (fontlib_GetCursorY() == (((SCR_HEIGHT - 1) * FONT_HEIGHT) + SCR_OFFSET_Y)) {
                    cursorY -= FONT_HEIGHT;
                    fontlib_ScrollWindowDown(); // Manually scroll window

                    // Scroll the contents of the screen buffer
                    memmove(&scrBuffer[0][0], &scrBuffer[1][0], (BUFFER_SIZE - SCR_WIDTH) * sizeof(char_styled_t));
                    for (k = 0; k < SCR_WIDTH; k++) {
                        scrBuffer[SCR_HEIGHT - 1][k].character = ' ';
                        scrBuffer[SCR_HEIGHT - 1][k].bold = false;
                        scrBuffer[SCR_HEIGHT - 1][k].italic = false;
                        scrBuffer[SCR_HEIGHT - 1][k].underline = false;
                        scrBuffer[SCR_HEIGHT - 1][k].fg_col = WHITE;
                        scrBuffer[SCR_HEIGHT - 1][k].bg_col = BLACK;
                    }

                    // Erase the bottom line
                    gfx_SetColor(BLACK);
                    gfx_FillRectangle(SCR_OFFSET_X, (((SCR_HEIGHT - 1) * FONT_HEIGHT) + SCR_OFFSET_Y), SCR_WIDTH_P, FONT_HEIGHT);

                    fontlib_SetCursorPosition(SCR_OFFSET_X, (((SCR_HEIGHT - 1) * FONT_HEIGHT) + SCR_OFFSET_Y));
                }
            }

            print_spaces(offsetX, cursorY, strlen(msg) + strlen(input) + 1);
            draw_str_update_buf(msg);
            
            // Make sure cursor offset does not exceed the size of input
            if (cursorOffset > strlen(input)) cursorOffset = strlen(input);

            // Draw stars if masking input, otherwise output plain text
            if (maskInput) {
                for (i = 1; i <= strlen(input) + 1; i++) {
                    if ((i == strlen(input) - cursorOffset + 1) && (j < 500)) {
                        temp[0] = CURSOR_INDEX[textIndex];
                        fontlib_DrawString(temp); // Draw cursor
                    } else if (i <= strlen(input)) {
                        draw_str_update_buf("*");
                    }
                }
            } else {
                i = strlen(input) - cursorOffset;
                temp[0] = input[i];
                if (j < 500) input[i] = CURSOR_INDEX[textIndex];
                if (!cursorOffset)
                    input[i + 1] = 0;
                draw_str_update_buf(input);
                input[i] = temp[0];
            }
            
            gfx_BlitBuffer();

        }
        
        // Suppress autoscroll
        shouldScroll = false;
        
    } while (!done);

    /* When user hits enter, update screen output without cursor */

    print_spaces(offsetX, cursorY, strlen(msg) + strlen(input) + 1);
    draw_str_update_buf(msg);

    // Draw stars if masking input, otherwise output plain text
    if (maskInput) {
        for (i = 1; i <= (int16_t)strlen(input); i++) {
            draw_str_update_buf("*");
        }
    } else {
        draw_str_update_buf(input);
    }
    
    gfx_BlitBuffer();
}

// Parse strings
void parse_draw_string(const char *string) {

    uint16_t i, j, x;
    uint8_t k, l, m, y;
    char temp[2] = {0, 0};
    bool displayNextChar,
         fGcolorHasBeenSet,
         bGcolorHasBeenSet;

    // Empty input
    for (i = 0; i <= INPUT_LENGTH; i++) {
        parserData[i] = 0;
    }

    displayNextChar = true;
    fGcolorHasBeenSet = false;
    bGcolorHasBeenSet = false;

    strcpy(parserData, string);

    // Begin displaying and parsing actual string
    for (i = 0; i < INPUT_LENGTH; i++) {

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
                if (fontlib_GetCursorX() == SCR_OFFSET_X) {
                    fontlib_SetCursorPosition(((SCR_WIDTH - 1) * FONT_WIDTH) + SCR_OFFSET_X, fontlib_GetCursorY() - FONT_HEIGHT);
                } else {
                    fontlib_SetCursorPosition(fontlib_GetCursorX() - FONT_WIDTH, fontlib_GetCursorY());
                }
                draw_str_update_buf(" ");
                if (fontlib_GetCursorX() == SCR_OFFSET_X) {
                    fontlib_SetCursorPosition(((SCR_WIDTH - 1) * FONT_WIDTH) + SCR_OFFSET_X, fontlib_GetCursorY() - FONT_HEIGHT);
                } else {
                    fontlib_SetCursorPosition(fontlib_GetCursorX() - FONT_WIDTH, fontlib_GetCursorY());
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
                fontlib_SetCursorPosition(SCR_OFFSET_X, fontlib_GetCursorY());
                displayNextChar = false;
                i++;
            }

            // Parse \t
            if (parserData[i + 1] == 't') {
                draw_str_update_buf("    ");
                if (underlineText) {
                    gfx_SetColor(fontlib_GetForegroundColor());
                    gfx_HorizLine(fontlib_GetCursorX() - FONT_WIDTH, fontlib_GetCursorY() + FONT_HEIGHT - 2, FONT_WIDTH);
                }
                displayNextChar = false;
                i++;
            }

            // Parse \𝘯𝘯𝘯
            if (inRange(parserData[i + 1],'0','7')) {

                // Increment m until the next character is not an octal digit or the sequence length is 3 digits
                for (m = i + 1; inRange(parserData[m],'0','7') && (m < i + 4); m++);

                // m now equals the length of the sequence
                m -= i + 1;

                // Replace last character with new character
                parserData[i + m] = str_to_num(&parserData[i + 1], m, 8);

                i += m;
            }

            // Parse \x𝘩𝘩
            if (parserData[i + 1] == 'x') {

                // Increment m until the next character is not a hex digit or the sequence length is 2 digits
                for (m = i + 2; (inRange(parserData[m],'0','9') || inRange(parserData[m],'A','F') || inRange(parserData[m],'a','f')) && (m < i + 4); m++);

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
                for (j = i + 3; j < INPUT_LENGTH; j++) {
                    if (inRange(parserData[j],'A','Z') || inRange(parserData[j],'a','z')) break;
                }

                // Move cursor up
                if (parserData[j] == 'A') {
                    fontlib_SetCursorPosition(fontlib_GetCursorX(), fontlib_GetCursorY() - (str_to_num(&parserData[i + 3], j - (i + 3), 10) * FONT_HEIGHT));
                }

                // Move cursor down
                if (parserData[j] == 'B') {
                    fontlib_SetCursorPosition(fontlib_GetCursorX(), fontlib_GetCursorY() + (str_to_num(&parserData[i + 3], j - (i + 3), 10) * FONT_HEIGHT));
                }

                // Move cursor forward
                if (parserData[j] == 'C') {
                    fontlib_SetCursorPosition(fontlib_GetCursorX() + (str_to_num(&parserData[i + 3], j - (i + 3), 10) * FONT_WIDTH), fontlib_GetCursorY());
                }

                // Move cursor backward
                if (parserData[j] == 'D') {
                    fontlib_SetCursorPosition(fontlib_GetCursorX() - (str_to_num(&parserData[i + 3], j - (i + 3), 10) * FONT_WIDTH), fontlib_GetCursorY());
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

                    fontlib_SetCursorPosition((x * FONT_WIDTH) + SCR_OFFSET_X, (y * FONT_HEIGHT) + SCR_OFFSET_Y);
                }

                // Clear screen
                if ((parserData[j] == 'J') && (parserData[j - 1] == '2')) {
                    fontlib_ClearWindow();
                    for (x = 0; x < SCR_WIDTH; x++) {
                        for (y = 0; y < SCR_HEIGHT; y++) {
                            scrBuffer[y][x].character = ' ';
                            scrBuffer[y][x].bold = false;
                            scrBuffer[y][x].italic = false;
                            scrBuffer[y][x].underline = false;
                            scrBuffer[y][x].fg_col = WHITE;
                            scrBuffer[y][x].bg_col = BLACK;
                        }
                    }
                }

                // Clear current line
                if (parserData[j] == 'K') {
                    x = fontlib_GetCursorX();
                    y = fontlib_GetCursorY();
                    fontlib_ClearEOL();
                    for (k = (x - SCR_OFFSET_X) / FONT_WIDTH; k < SCR_WIDTH; k++) {
                        scrBuffer[y][k].character = ' ';
                        scrBuffer[y][k].bold = false;
                        scrBuffer[y][k].italic = false;
                        scrBuffer[y][k].underline = false;
                        scrBuffer[y][k].fg_col = WHITE;
                        scrBuffer[y][k].bg_col = BLACK;
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
                            fontlib_SetColors(WHITE, BLACK);
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
                        if (parserData[l - 2] == '3') {
                            fGcolorHasBeenSet = true;
                            switch (parserData[l - 1]) {
                                case '0':
                                    fontlib_SetForegroundColor(BLACK);
                                    break;
                                case '1':
                                    fontlib_SetForegroundColor(RED);
                                    break;
                                case '2':
                                    fontlib_SetForegroundColor(GREEN);
                                    break;
                                case '3':
                                    fontlib_SetForegroundColor(YELLOW);
                                    break;
                                case '4':
                                    fontlib_SetForegroundColor(BLUE);
                                    break;
                                case '5':
                                    fontlib_SetForegroundColor(PURPLE);
                                    break;
                                case '6':
                                    fontlib_SetForegroundColor(CYAN);
                                    break;
                                case '7':
                                    fontlib_SetForegroundColor(WHITE);
                                    break;
                                default:
                                    fGcolorHasBeenSet = false;
                                    break;
                            }
                        }

                        // Colored background text
                        if (parserData[l - 2] == '4') {
                            bGcolorHasBeenSet = true;
                            switch (parserData[l - 1]) {
                                case '0':
                                    fontlib_SetBackgroundColor(BLACK);
                                    break;
                                case '1':
                                    fontlib_SetBackgroundColor(RED);
                                    break;
                                case '2':
                                    fontlib_SetBackgroundColor(GREEN);
                                    break;
                                case '3':
                                    fontlib_SetBackgroundColor(YELLOW);
                                    break;
                                case '4':
                                    fontlib_SetBackgroundColor(BLUE);
                                    break;
                                case '5':
                                    fontlib_SetBackgroundColor(PURPLE);
                                    break;
                                case '6':
                                    fontlib_SetBackgroundColor(CYAN);
                                    break;
                                case '7':
                                    fontlib_SetBackgroundColor(WHITE);
                                    break;
                                default:
                                    bGcolorHasBeenSet = false;
                                    break;
                            }
                        }
                    }
                } while (l > i + 2);

                displayNextChar = false;
                i = j;
            }
        }
        if (parserData[i] == 0) break;

        // Output next character
        if (displayNextChar) {
            temp[0] = parserData[i];
            draw_str_update_buf(temp);
        } else {
            displayNextChar = true;
        }
    }
}

// Draw a string and update the screen buffer
void draw_str_update_buf(const char *string) {

    uint16_t i, j;
    uint8_t x, y;
    char temp[2] = {0, 0};

    // Loop through input
    for (i = 0; i < INPUT_LENGTH; i++) {
        if (string[i] == 0) break;

        // Get x & y coords
        x = (fontlib_GetCursorX() - SCR_OFFSET_X) / FONT_WIDTH;
        y = (fontlib_GetCursorY() - SCR_OFFSET_Y) / FONT_HEIGHT;

        // If the screen should scroll...
        if (shouldScroll) {
            fontlib_ScrollWindowDown(); // Manually scroll window

            // Scroll the contents of the screen buffer
            memmove(&scrBuffer[0][0], &scrBuffer[1][0], (BUFFER_SIZE - SCR_WIDTH) * sizeof(char_styled_t));
            for (j = 0; j < SCR_WIDTH; j++) {
                scrBuffer[SCR_HEIGHT - 1][j].character = ' ';
                scrBuffer[SCR_HEIGHT - 1][j].bold = false;
                scrBuffer[SCR_HEIGHT - 1][j].italic = false;
                scrBuffer[SCR_HEIGHT - 1][j].underline = false;
                scrBuffer[SCR_HEIGHT - 1][j].fg_col = WHITE;
                scrBuffer[SCR_HEIGHT - 1][j].bg_col = BLACK;
            }

            // Erase the bottom line
            gfx_SetColor(BLACK);
            gfx_FillRectangle(SCR_OFFSET_X, (((SCR_HEIGHT - 1) * FONT_HEIGHT) + SCR_OFFSET_Y), SCR_WIDTH_P, FONT_HEIGHT);

            fontlib_SetCursorPosition(SCR_OFFSET_X, (((SCR_HEIGHT - 1) * FONT_HEIGHT) + SCR_OFFSET_Y));

            x = 0;
            y = SCR_HEIGHT - 1;
        }

        shouldScroll = ((x == SCR_WIDTH - 1) && (y == SCR_HEIGHT - 1));

        temp[0] = string[i];
        fontlib_DrawString(temp);

        if (underlineText) {
            gfx_SetColor(fontlib_GetForegroundColor());
            gfx_HorizLine(fontlib_GetCursorX() - FONT_WIDTH, fontlib_GetCursorY() + FONT_HEIGHT - 2, FONT_WIDTH);
        }

        scrBuffer[y][x].character = string[i];
        scrBuffer[y][x].bold = boldText;
        scrBuffer[y][x].italic = italicText;
        scrBuffer[y][x].underline = underlineText;
        scrBuffer[y][x].fg_col = fontlib_GetForegroundColor();
        scrBuffer[y][x].bg_col = fontlib_GetBackgroundColor();
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

    if (((y - SCR_OFFSET_Y) / FONT_HEIGHT) == SCR_HEIGHT - 1) {
        fontlib_ScrollWindowDown(); // Manually scroll window

        // Scroll the contents of the screen buffer
        memmove(&scrBuffer[0][0], &scrBuffer[1][0], (BUFFER_SIZE - SCR_WIDTH) * sizeof(char_styled_t));
        for (j = 0; j < SCR_WIDTH; j++) {
            scrBuffer[SCR_HEIGHT - 1][j].character = ' ';
            scrBuffer[SCR_HEIGHT - 1][j].bold = false;
            scrBuffer[SCR_HEIGHT - 1][j].italic = false;
            scrBuffer[SCR_HEIGHT - 1][j].underline = false;
            scrBuffer[SCR_HEIGHT - 1][j].fg_col = WHITE;
            scrBuffer[SCR_HEIGHT - 1][j].bg_col = BLACK;
        }

        // Erase the bottom line
        gfx_SetColor(BLACK);
        gfx_FillRectangle(SCR_OFFSET_X, (((SCR_HEIGHT - 1) * FONT_HEIGHT) + SCR_OFFSET_Y), SCR_WIDTH_P, FONT_HEIGHT);

        fontlib_SetCursorPosition(SCR_OFFSET_X, y);
    } else {
        fontlib_Newline();
    }
}

// Print spaces. That's literally it
void print_spaces(uint16_t x, uint16_t y, uint16_t num) {
    
    uint16_t i;
    
    if (x) {
        fontlib_SetCursorPosition(x, y);
    } else {
        fontlib_SetCursorPosition(SCR_OFFSET_X, y);
    }
    for (i = 0; i < num; i++) {
        fontlib_DrawString(" ");
    }
    if (x) {
        fontlib_SetCursorPosition(x, y);
    } else {
        fontlib_SetCursorPosition(SCR_OFFSET_X, y);
    }
}

// Convert any base (up to base 36) string literal to 8-bit decimal integer
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

// Get GetCSC codes using keypadc; code by jacobly
uint8_t get_single_key_pressed(void) {
    static uint8_t last_key;
    uint8_t only_key = 0;
    kb_Scan();
    for (uint8_t key = 1, group = 7; group; --group) {
        for (uint8_t mask = 1; mask; mask <<= 1, ++key) {
            if (kb_Data[group] & mask) {
                if (only_key) {
                    last_key = 0;
                    return 0;
                } else {
                    only_key = key;
                }
            }
        }
    }
    if (only_key == last_key) {
        return 0;
    }
    last_key = only_key;
    return only_key;
}

// Save the shell state and runs a program
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
    sdUnderlineText = underlineText;
    sdItalicText = italicText;
    sdBoldText = boldText;

    // Save shell state
    settingsAppvar = ti_Open("CEshSett", "r+");

    ti_Seek(sizeof(char) * (2 * USER_PWD_LENGTH), SEEK_SET, settingsAppvar);
    ti_Write(&sdRetFromPrgm, sizeof(bool), 1, settingsAppvar);
    ti_Write(&sdCursorPos, sizeof(uint16_t), 2, settingsAppvar);
    ti_Write(&sdTextColors, sizeof(uint8_t), 2, settingsAppvar);
    ti_Seek(sizeof(uint16_t) * 7, SEEK_SET, settingsAppvar);
    ti_Write(&sdUnderlineText, sizeof(bool), 1, settingsAppvar);
    ti_Write(&sdItalicText, sizeof(bool), 1, settingsAppvar);
    ti_Write(&sdBoldText, sizeof(bool), 1, settingsAppvar);

    ti_SetArchiveStatus(true, settingsAppvar);
    ti_Close(settingsAppvar);

    // Save screen state
    settingsAppvar = ti_Open("CEshSBuf", "w+");
    ti_Write(&scrBuffer, sizeof(char_styled_t), BUFFER_SIZE, settingsAppvar);
    ti_Close(settingsAppvar);

    gfx_End();

    ret = os_RunPrgm(prgm, NULL, 0, (os_runprgm_callback_t)main);
    cesh_Init();

    return ret;
}