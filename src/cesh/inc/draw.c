#include <string.h>

#include <graphx.h>
#include <fontlibc.h>
#include <fileioc.h>
#include <keypadc.h>

#include "fonts/fonts.h"

#include "globals.h"
#include "macros.h"
#include "types.h"
#include "routines.h"

#include "draw.h"

void parse_draw_string(const char *string) {

    static uint16_t cursorPos[2] = {SCR_OFFSET_X, SCR_OFFSET_Y};
    uint16_t i, j, x;
    uint8_t k, l, m, y;
    char temp[2] = {0, 0},
         parserData[INPUT_LENGTH];
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

                    // Store index of ';' in m
                    for (m = i + 3; m < j; m++) {
                        if (parserData[m] == ';')
                            break;
                    }

                    // Move cursor to top-left (0,0)
                    if (parserData[j - 1] == '[') {
                        x = 0;
                        y = 0;

                    // Retrieve & set cursor position
                    } else {
                        x = str_to_num(&parserData[m + 1], j - (m + 1), 10);
                        y = str_to_num(&parserData[i + 3], m - (i + 3), 10);
                    }

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

void print_spaces(uint16_t x, uint16_t y, uint16_t num) {

    uint16_t i;

    if (x) {
        fontlib_SetCursorPosition(x, y);
    } else {
        fontlib_SetCursorPosition(SCR_OFFSET_X, y);
    }
    for (i = 0; i < num; i++) {
        scrBuffer[(fontlib_GetCursorY() - SCR_OFFSET_Y) / FONT_HEIGHT][(fontlib_GetCursorX() - SCR_OFFSET_X) / FONT_WIDTH].character = ' ';
        scrBuffer[(fontlib_GetCursorY() - SCR_OFFSET_Y) / FONT_HEIGHT][(fontlib_GetCursorX() - SCR_OFFSET_X) / FONT_WIDTH].bold = false;
        scrBuffer[(fontlib_GetCursorY() - SCR_OFFSET_Y) / FONT_HEIGHT][(fontlib_GetCursorX() - SCR_OFFSET_X) / FONT_WIDTH].italic = false;
        scrBuffer[(fontlib_GetCursorY() - SCR_OFFSET_Y) / FONT_HEIGHT][(fontlib_GetCursorX() - SCR_OFFSET_X) / FONT_WIDTH].underline = false;
        scrBuffer[(fontlib_GetCursorY() - SCR_OFFSET_Y) / FONT_HEIGHT][(fontlib_GetCursorX() - SCR_OFFSET_X) / FONT_WIDTH].fg_col = WHITE;
        scrBuffer[(fontlib_GetCursorY() - SCR_OFFSET_Y) / FONT_HEIGHT][(fontlib_GetCursorX() - SCR_OFFSET_X) / FONT_WIDTH].bg_col = BLACK;
        fontlib_DrawString(" ");
    }
    if (x) {
        fontlib_SetCursorPosition(x, y);
    } else {
        fontlib_SetCursorPosition(SCR_OFFSET_X, y);
    }
}
