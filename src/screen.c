#include "screen.h"
#include "cache.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>


int faye_fdout;
FILE* faye_out;
SCREEN* faye_screen;
struct content julia;


void print_err(const char* format, ...){
	int closed = 0;
	if(!isendwin()){
		endwin();
		closed = 1;
	}
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	fflush(stderr);
	if(closed){
		refresh();
	}
}


void initialize_content(struct content* c){
	c->first = 0;
	c->max = LINES-4;
	c->show_hidden = 0;
	c->update = 1;
}


int set_output(){
	faye_fdout = fileno(stdout);	
	faye_out = stdout;
	if(!isatty(faye_fdout)){
		faye_fdout = open("/dev/tty", O_WRONLY);
		if(faye_fdout < 0){
			return -1;
		}
		faye_out = fdopen(faye_fdout, "w");
		if(!faye_out){
			close(faye_fdout);
			return -2;
		}
	}
	return 0;
}


int start_screen(){
	if(set_output() < 0){
		return -1;
	}
	faye_screen = newterm(NULL, faye_out, stdin);
	return 0;
}


void unset_output(){
	int fd_out = fileno(stdout);
	if(faye_fdout != fd_out){
		close(faye_fdout);
		faye_fdout = fd_out;
		fclose(faye_out);
		faye_out = stdout;
	}
}


void clear_lines(int line_count, int x, int y){
	int prev_x, prev_y;
	getyx(stdscr, prev_y, prev_x);
	for(int i = 0; i < line_count; i++){
		move(y+i, x);
		clrtoeol();
	}
	move(prev_y, prev_x);
	refresh();
}


void print_lines(int x, int y){
	int prev_x, prev_y;
	int line = 0;
	int printable = 0;
	char hold;
	getyx(stdscr, prev_y, prev_x); 
	clear_lines(julia.max, x, y);
	for(int i = 0; i < ein.files; i++){
		if(!julia.show_hidden && ein.filenames[i][0] == '.'){
			continue;
		}
		if(printable >= julia.first && line < julia.max){
			hold = ein.filenames[i][COLS]; 
			ein.filenames[i][COLS] = 0;
			mvprintw(y+line, x, ein.filenames[i]);	
			ein.filenames[i][COLS] = hold;
			line++;
		}
		printable++;
	}
	move(prev_y, prev_x);
	refresh();
}


void clean_bookmarks(int x, int y){
	for(int i = 0; i < julia.max; i++){
		move(y+i, 0);
		for(int j = 0; j < x; j++){
			addch(' '); 
		}
	}
	refresh();
}


void draw_bookmarks(int x, int y){
	int relative;
	clean_bookmarks(x, y);
	for(int i = 0; i < ein.bookmark_count; i++){
		relative = get_relative(ein.bookmarks[i], julia.show_hidden);
		if(relative >= julia.first && relative < julia.first + julia.max){
			mvprintw(y+relative-julia.first, x/2, "*");
		}
	}
	refresh();
}


void redraw(int x, int y){
	draw_bookmarks(x, y);
	if(!julia.update){
		return;
	}
	print_lines(x, y);	
	julia.update = 0;
}
