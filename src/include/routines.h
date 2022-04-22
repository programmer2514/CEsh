#ifndef CESH_ROUTINES
#define CESH_ROUTINES

#include "globals.h"
#include "macros.h"
#include "types.h"
#include "main.h"
#include "input.h"

// Convert any base (up to base 36) string literal to 8-bit decimal integer
uint16_t str_to_num(const char *string, const uint8_t length, const uint8_t base);

int run_prgm(char *prgm, char *args);
void power_down(bool restart, bool save);

#endif