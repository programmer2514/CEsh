#include <string.h>

#include <fileioc.h>

#include "inc/shell.h"
#include "inc/macros.h"
#include "inc/globals.h"

#include "cesh.h"

void cesh_Main(void) {
    sh_main();
}

void cesh_Init(void) {
    sh_init();
}

uint8_t cesh_GetNumArgs(void) {

    uint8_t numargs;

    appvarSlot = ti_Open("CEshArgs", "r");
    if (appvarSlot == 0)
        return 0;

    ti_Seek(INPUT_LENGTH, SEEK_SET, appvarSlot);
    ti_Read(&numargs, sizeof(uint8_t), 1, appvarSlot);

    ti_Close(appvarSlot);

    return numargs;
}

void cesh_GetArg(uint8_t index, char *data) {

    uint8_t argloc;

    appvarSlot = ti_Open("CEshArgs", "r");

    if (appvarSlot != 0) {

        ti_Seek(INPUT_LENGTH + sizeof(uint8_t) + index, SEEK_SET, appvarSlot);
        ti_Read(&argloc, sizeof(uint8_t), 1, appvarSlot);

        ti_Seek(argloc, SEEK_SET, appvarSlot);
        const char *direct_data = (char *)ti_GetDataPtr(appvarSlot);
        strcpy(data, direct_data);

        ti_Close(appvarSlot);
    }
}