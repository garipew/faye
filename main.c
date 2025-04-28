#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>


#define PATH_MAX 4096
#define FILE_MAX 255
#define COLS 30
#define LINES 20
#define DIR_MAX 20


char path[PATH_MAX];
int path_next = 0;
int selected = 0;
DIR* dir_buffer[DIR_MAX];
int depth;
char last_file[FILE_MAX];
int last_len;


int ls(){
	struct dirent *file;
	int count = 0;
	while((file = readdir(dir_buffer[depth-1])) != NULL){
		if(file->d_name[0] == '.'){
			continue;
		}
		printw(" [%c] %s\n", selected == count ? 'x' : ' ', file->d_name);
		count++;
	}
	rewinddir(dir_buffer[depth-1]);
	return count;
}


void get_hover(char* hover){
	struct dirent *file;
	int count = 0;
	while((file = readdir(dir_buffer[depth-1])) != NULL){
		if(file->d_name[0] == '.'){
			continue;
		}
		if(count == selected){
			break;
		}
		count++;
	}
	rewinddir(dir_buffer[depth-1]);
	strncpy(hover, file->d_name, FILE_MAX);
	hover[FILE_MAX-1] = 0;
}

//tries to open str and update path
void try_open(char* str, size_t str_len){
	if(str_len + path_next + 1 < PATH_MAX){
		strcat(path, str);
	}
	if(depth >= DIR_MAX){
		return;
	}
	dir_buffer[depth] = opendir(path);
	if(!dir_buffer[depth]){
		path[path_next] = 0;
		return;
	}
	selected = 0;
	depth++;
	path_next += str_len;
	strcpy(last_file, str);
	last_len = str_len;
	path[path_next++] = '/';
	path[path_next] = 0;
}


// open hovering directory
void open_hover(){
	char hover[FILE_MAX];
	size_t hover_len;
	get_hover(hover);
	hover_len = strlen(hover);
	
	try_open(hover, hover_len);
}


// go back one directory, tries to exhaust path buffer first
void move_backwards(){
	char* prev;
	if(depth > 1 && (strcmp(last_file, "..") != 0)){
	// more than one file opened and last was not a back move
		closedir(dir_buffer[--depth]);
		path[--path_next] = 0;
		prev = strrchr(path, '/');
		if(!prev){
			strcpy(last_file, ".");
			last_len = 1;
			return;
		}
		strcpy(last_file, prev+1);
		last_len = strlen(last_file);
		selected = 0;
		path_next -= last_len;
		path[path_next] = 0;
	} else{
	// one file open or last was a back move
		try_open("..", 2);
	}

}


void move_cursor(int direction, int max){
	switch(direction){
		case 'j':
			if(selected < max-1){
				selected++;
			}
			break;
		case 'k':
			if(selected > 0){
				selected--;
			}
			break;
		case 'l':
			open_hover();
			break;
		case 'h':
			move_backwards();
			break;
		default:
			return;
	}
}


int main(int argc, char** argv){
	int action;
	char start_path[PATH_MAX] = {0};
	size_t start_len;

	start_path[0] = '.';
	if(argc > 1){
		strncpy(start_path, argv[1], sizeof(path));
		start_path[PATH_MAX-1] = 0;
	}
	start_len = strnlen(start_path, PATH_MAX);

	try_open(start_path, start_len);

	initscr();
	noecho();
	cbreak();
	//WINDOW* win = newwin(LINES, COLS, 0, 0);

	int file_count = ls();
	while((action = getch()) != 'q'){
		clear();
		move_cursor(action, file_count);
		file_count = ls();
		mvprintw(LINES, 0, path);
		refresh();
	}

	for(int i = 0; i < depth; i++){
		closedir(dir_buffer[i]);
	}
	endwin();
	return 0;
}
