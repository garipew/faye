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
	char* path = NULL;
	if(argc > 1){
		path = argv[1];
	}

	if(!initialize_cmd(&ed)){
		endwin();
		delscreen(faye_screen);
		return 1;
	}
	initialize_cache(&ein);
	initialize_navigator(&jet, path);
	initialize_content(&julia);

	if(open_path() != 0){
		free_cmd(&ed);
		endwin();
		delscreen(faye_screen);
		return 1;
	}

	noecho();
	cbreak();

	load_files();
	int file_count = count_printable();
	do{
		file_count = update(action, file_count);
	}while((action = getch()) != 'q');

	endwin();
	delscreen(faye_screen);

	free_cache(&ein);
	free_cmd(&ed);
	if(faye_out != stdout){
		fprintf(stdout, jet.cwd);
	}
	unset_output();
	return 0;
}
