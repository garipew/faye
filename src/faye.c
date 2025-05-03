#include "navigation.h"
#include "screen.h"
#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char** argv){
	if(start_screen() < 0){
		return -1;
	}
	int action;

	if(!initialize_cmd(&ed)){
		return 1;
	}
	initialize_cache(&ein);
	initialize_navigator(&jet);

	getcwd(jet.cwd, FAYE_PATH_MAX);
	if(argc > 1){
		filter_input(argv[1], jet.cwd);
	}
	jet.cwd_len = strnlen(jet.cwd, FAYE_PATH_MAX);
	if(jet.cwd_len < FAYE_PATH_MAX-1 && jet.cwd[jet.cwd_len-1] != '/'){
		jet.cwd[jet.cwd_len++] = '/';
		jet.cwd[jet.cwd_len] = 0;
	}

	open_path();
	if(!ein.dir_buffer[0].d_file){
		fprintf(stderr, "%s: Unable to open %s\n", argv[0], jet.cwd);
		return 1;
	}

	noecho();
	cbreak();

	int file_count = ls();
	do{
		file_count = update(action, file_count);
		refresh();
	}while((action = getch()) != 'q');

	endwin();
	delscreen(faye_screen);

	while(--ein.next >= 0){
		closedir(ein.dir_buffer[ein.next].d_file);
	}
	free(ed.buffer);
	if(faye_out != stdout){
		fprintf(stdout, jet.cwd);
	}
	unset_output();
	return 0;
}
