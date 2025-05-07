#ifndef FAYE_SCREEN
#define FAYE_SCREEN

#include <stddef.h>
#include <ncurses.h>


#define FAYE_PATH_MAX 4096
#define FAYE_FILE_MAX 255
#define FAYE_MAX 20
#define FAYE_COLS 30
#define FAYE_LINES 20


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
void redraw(char**, int, int, int);
#endif
