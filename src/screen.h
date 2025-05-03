#ifndef FAYE_SCREEN
#define FAYE_SCREEN

#include <stddef.h>
#include <ncurses.h>


extern FILE* faye_out;
extern SCREEN* faye_screen;

int start_screen();
void unset_output();
#endif
