#ifndef CESH_SHELL
#define CESH_SHELL

void sh_main(void); // The main shell function
void sh_init(void); // Initializes shell data and settings
void sh_setup(void); // First time setup routine
void sh_splash(void); // Opening splash screen (TODO: merge with sh_init)
void sh_shell(void); // Main shell loop
void sh_pregc(void); // Cleans up prior to a GarbageCollect
void sh_end(void); // Saves and exits shell

#endif