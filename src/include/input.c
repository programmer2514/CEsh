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

#include "input.h"

void parse_user_input(void) {

    uint16_t i, j, k;
    char *ptr;
    int retval, size;
    uint8_t numargs = 1;
    uint8_t *arglocs = malloc(1);
    bool inQuotes = false;

    appvarSlot = ti_Open("CEshHist", "r+");
    if (!appvarSlot)
        appvarSlot = ti_Open("CEshHist", "w+");
    size = ti_GetSize(appvarSlot);

    // If it has fewer than 250 entries, seek to the end of the file
    if (size < (250 * INPUT_LENGTH)) {
        ti_Seek(0, SEEK_END, appvarSlot);

    // Otherwise, delete the oldest entry
    } else {
        ptr = ti_GetDataPtr(appvarSlot);
        memmove(ptr, ptr + INPUT_LENGTH, size - INPUT_LENGTH);
        ti_Seek(size - INPUT_LENGTH, SEEK_SET, appvarSlot);
    }

    // Add command to history buffer & close appvar
    ti_Write(&input, sizeof(char), INPUT_LENGTH, appvarSlot);
    ti_Close(appvarSlot);

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

    // Command: logout
    } else if (!strcmp(input, "logout")) {

        isRetFromPrgm = false;
        appvarSlot = ti_Open("CEshSett", "r+");
        ti_Seek(2 * USER_PWD_LENGTH, SEEK_SET, appvarSlot);
        ti_Write(&isRetFromPrgm, sizeof(bool), 1, appvarSlot);
        ti_SetArchiveStatus(true, appvarSlot);
        ti_Close(appvarSlot);
        main();

    // Command: reboot
    } else if (!strcmp(input, "reboot")) {

        isRetFromPrgm = false;
        appvarSlot = ti_Open("CEshSett", "r+");
        ti_Seek(2 * USER_PWD_LENGTH, SEEK_SET, appvarSlot);
        ti_Write(&isRetFromPrgm, sizeof(bool), 1, appvarSlot);
        ti_SetArchiveStatus(true, appvarSlot);
        ti_Close(appvarSlot);
        power_down(true, false);
        main();

    // Command: shutdown
    } else if (!strcmp(input, "shutdown")) {

        isRetFromPrgm = false;
        appvarSlot = ti_Open("CEshSett", "r+");
        ti_Seek(2 * USER_PWD_LENGTH, SEEK_SET, appvarSlot);
        ti_Write(&isRetFromPrgm, sizeof(bool), 1, appvarSlot);
        ti_SetArchiveStatus(true, appvarSlot);
        ti_Close(appvarSlot);
        if (numargs > 1) {
            if (!strcmp(&input[arglocs[1]], "-n")) {
                power_down(false, false);
                cesh_End();
            } else if (!strcmp(&input[arglocs[1]], "-r")) {
                power_down(true, false);
                main();
            }
        } else {
            power_down(false, false);
            main();
        }

    // Command: history
    } else if (!strcmp(input, "history")) {

        startOnNewLine = false;
        draw_newline();

        appvarSlot = ti_Open("CEshHist", "r");
        size = ti_GetSize(appvarSlot);

        if (numargs > 1) {
            if (!strcmp(&input[arglocs[1]], "-c")) {
                ti_Delete("CEshHist");
            } else if ((numargs > 2) && (!strcmp(&input[arglocs[1]], "-d"))) {
                ptr = ti_GetDataPtr(appvarSlot);
                j = str_to_num(&input[arglocs[2]], strlen(&input[arglocs[2]]), 10);
                memmove(ptr + INPUT_LENGTH, ptr, (j - 1) * INPUT_LENGTH);
                ti_Resize(size - INPUT_LENGTH, appvarSlot);
            } else {
                j = str_to_num(&input[arglocs[1]], strlen(&input[arglocs[1]]), 10);
                for (i = (size / INPUT_LENGTH) - j; i < (size / INPUT_LENGTH); i++) {
                    ti_Seek(i * INPUT_LENGTH, SEEK_SET, appvarSlot);
                    ti_Read(&input, sizeof(char), INPUT_LENGTH, appvarSlot);

                    draw_int_update_buf(i + 1, 1 + ((i + 1) >= 10) + ((i + 1) >= 100));
                    draw_str_update_buf("  ");
                    draw_str_update_buf(input);
                    parse_draw_string("\\n");
                }
            }
        } else {
            for (i = 0; i < (size / INPUT_LENGTH); i++) {
                ti_Seek(i * INPUT_LENGTH, SEEK_SET, appvarSlot);
                ti_Read(&input, sizeof(char), INPUT_LENGTH, appvarSlot);

                draw_int_update_buf(i + 1, 1 + ((i + 1) >= 10) + ((i + 1) >= 100));
                draw_str_update_buf("  ");
                draw_str_update_buf(input);
                parse_draw_string("\\n");
            }
        }

        ti_Close(appvarSlot);

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

        // Handle poweroff
        if (kb_On) {
            kb_ClearOnLatch();
            if (textIndex == 0) {
                textIndex = 3;
                power_down(false, true);
            }
        }

        // Blink Cursor
        if (j == 0) { // Update screen output w/ cursor

            print_spaces(offsetX, cursorY, strlen(msg) + strlen(input) + 1);
            draw_str_update_buf(msg);

            // Draw stars if masking input, otherwise output plain text
            for (i = 0; i <= strlen(input); i++) {
                if (i == (strlen(input) - cursorOffset)) {
                    temp[0] = CURSOR_INDEX[textIndex];
                    fontlib_DrawString(temp); // Draw cursor
                } else if (i <= strlen(input)) {
                    if (maskInput) {
                        draw_str_update_buf("*");
                    } else {
                        temp[0] = input[i];
                        fontlib_DrawString(temp);
                    }
                }
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
                            appvarSlot = ti_Open("CEshHist", "r");
                            if (appvarSlot && histOffset) {
                                ti_Seek(0 - (histOffset * INPUT_LENGTH), SEEK_END, appvarSlot);
                                ti_Read(&input, sizeof(char), INPUT_LENGTH, appvarSlot);
                            }
                            ti_Close(appvarSlot);
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
                            appvarSlot = ti_Open("CEshHist", "r");
                            if (appvarSlot) {
                                ti_Seek(0 - (histOffset * INPUT_LENGTH), SEEK_END, appvarSlot);
                                ti_Read(&input, sizeof(char), INPUT_LENGTH, appvarSlot);
                            }
                            histOffset = (histOffset + 1) * (histOffset <= (ti_GetSize(appvarSlot) / INPUT_LENGTH));
                            ti_Close(appvarSlot);
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
                        if (textIndex < 2)
                            cesh_End();
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
            for (i = 0; i <= strlen(input); i++) {
                if (i == (strlen(input) - cursorOffset)) {
                    temp[0] = CURSOR_INDEX[textIndex];
                    fontlib_DrawString(temp); // Draw cursor
                } else if (i <= strlen(input)) {
                    if (maskInput) {
                        draw_str_update_buf("*");
                    } else {
                        temp[0] = input[i];
                        fontlib_DrawString(temp);
                    }
                }
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