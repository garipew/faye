#include "fdir.h"
#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char** argv){
	int action;

	getcwd(path, FDIR_PATH_MAX);
	if(argc > 1){
		filter_input(argv[1], path);
	}
	path_next = strnlen(path, FDIR_PATH_MAX);

	open_path();
	if(!dir_buffer[0].d_file){
		fprintf(stderr, "%s: Unable to open %s\n", argv[0], path);
		return 1;
	}

	initscr();
	noecho();
	cbreak();

	int file_count = ls();
	do{
		file_count = update(action, file_count);
		refresh();
	}while((action = getch()) != 'q');

	endwin();

	while(--depth >= 0){
		closedir(dir_buffer[depth].d_file);
	}
	return 0;
}
