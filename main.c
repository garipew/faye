#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>


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
int show_hidden = 0;


int ls(){
	struct dirent *file;
	int count = 0;
	while((file = readdir(dir_buffer[depth-1])) != NULL){
		if(!show_hidden && file->d_name[0] == '.'){
			continue;
		}
		if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, "..")){
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
		if(!show_hidden && file->d_name[0] == '.'){
			continue;
		}
		if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, "..")){
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
	if(path[path_next-1] != '/'){
		path[path_next++] = '/';
		path[path_next] = 0;
	}
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


void update(int direction, int max){
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
		case 's':
			show_hidden = !show_hidden;
			break;
		default:
			return;
	}
}


void filter_input(char* input, char* buffer){
	if(input[0] == '/'){
		strncpy(buffer, input, sizeof(path));
		buffer[PATH_MAX-1] = 0;
	} else{
		strcat(buffer, "/");
		strcat(buffer, input);
	}
}


int main(int argc, char** argv){
	int action;
	char start_path[PATH_MAX] = {0};
	size_t start_len;

	getcwd(start_path, PATH_MAX);
	if(argc > 1){
		filter_input(argv[1], start_path);
	}
	start_len = strnlen(start_path, PATH_MAX);

	try_open(start_path, start_len);
	if(path_next == 0){
		fprintf(stderr, "%s: Unable to open %s\n", argv[0], start_path);
		return 1;
	}

	initscr();
	noecho();
	cbreak();

	int file_count = ls();
	mvprintw(LINES, 0, path);
	while((action = getch()) != 'q'){
		clear();
		update(action, file_count);
		file_count = ls();
		mvprintw(LINES, 0, path);
		refresh();
	}

	endwin();

	while(--depth >= 0){
		closedir(dir_buffer[depth]);
	}
	return 0;
}
