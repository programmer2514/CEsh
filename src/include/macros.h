#ifndef CESH_MACROS
#define CESH_MACROS

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

#endif