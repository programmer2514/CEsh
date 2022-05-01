#ifndef _CESHLIB_H
#define _CESHLIB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Calls the main CEsh shell loop
 *
 * @warning Don't use this unless you know exactly what you're doing.
 *
 * @returns NULL
 */
void cesh_Main(void);

/**
 * Initializes the CEsh UI, useful for command line utilities
 *
 * @returns NULL
 */
void cesh_Init(void);

#ifdef __cplusplus
}
#endif

#endif