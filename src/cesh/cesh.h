#ifndef _CESHLIB_H
#define _CESHLIB_H

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