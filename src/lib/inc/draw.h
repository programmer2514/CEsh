#ifndef CESH_DRAW
#define CESH_DRAW

void parse_draw_string(const char *string);
void draw_str_update_buf(const char *string);
void draw_int_update_buf(int number, const uint8_t length);
void draw_newline(void);
void print_spaces(uint16_t x, uint16_t y, uint16_t num);

#endif