#ifndef FAYE_SCREEN
#define FAYE_SCREEN

#include <stddef.h>
#include <ncurses.h>


#define FAYE_PATH_MAX 4096
#define FAYE_FILE_MAX 255
#define FAYE_MAX 20
#define FAYE_COLS 30
#define FAYE_LINES 20


extern FILE* faye_out;
extern SCREEN* faye_screen;

int start_screen();
void unset_output();
#endif
