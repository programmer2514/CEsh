#ifndef _CESHLIB_H
#define _CESHLIB_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Starts the CEsh main loop, equivalent to executing CESH.8xp.
 */
void cesh_Main(void);

/**
 * Initializes the CEsh UI, useful for command line utilities.
 */
void cesh_Init(void);

/**
 * Returns the number of arguments passed to the program by CEsh.
 *
 * @returns number of arguments passed, 0 if not called by CEsh
 */
uint8_t cesh_GetNumArgs(void);

/**
 * Fetches an argument passed to the program by CEsh at a specified index.   <br>
 * Index 0 is the command used to invoke the program.
 *
 * @param[in] index Index of the argument to be fetched
 * @param[out] data Address to read argument data into
 *
 * @warning Make sure enough memory is allocated in <tt>data</tt> for the argument to be read. Displays undefined behavior if the argument does not exist
 */
void cesh_GetArg(uint8_t index, char *data);

#ifdef __cplusplus
}
#endif

#endif