#ifndef CESH_INPUT
#define CESH_INPUT

#include "globals.h"
#include "macros.h"
#include "types.h"
#include "draw.h"
#include "routines.h"

void parse_user_input(void);
void get_user_input(const char *msg, const bool maskInput, const bool disableRecall, const uint16_t offsetX);
uint8_t get_single_key_pressed(void); // Get GetCSC codes using keypadc

#endif