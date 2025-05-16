#ifndef FAYE_SCREEN
#define FAYE_SCREEN

#include <stddef.h>
#include <ncurses.h>


struct content{
	int first;
	int max;
	int show_hidden;
	int update;
};


extern FILE* faye_out;
extern SCREEN* faye_screen;
extern struct content julia;

void print_err(const char*, ...);
void initialize_content(struct content*);
int start_screen();
void unset_output();
void clear_lines(int, int, int);
void redraw(int, int);
int get_absolute(int, int);
int get_relative(int, int);
#endif
