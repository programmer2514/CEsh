#ifndef CESH_H
#define CESH_H

#ifdef __cplusplus
extern "C" {
#endif

void cesh_Main(void);
void cesh_Init(void);
void cesh_Setup(void);
void cesh_Splash(void);
void cesh_Shell(void);
void cesh_PreGC(void);
void cesh_End(void);

#ifdef __cplusplus
}
#endif

#endif