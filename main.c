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


struct directory{
	DIR* d_file;
	char path[PATH_MAX];
};

char path[PATH_MAX];
int path_next = 0;
int selected = 0;
struct directory dir_buffer[DIR_MAX];
int depth = -1; // where you are on the buffer
int dir_count; // where the next directory will be placed on buffer
int show_hidden = 0;


int ls(){
	struct dirent *file;
	int count = 0;
	while((file = readdir(dir_buffer[depth].d_file)) != NULL){
		if(!show_hidden && file->d_name[0] == '.'){
			continue;
		}
		if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, "..")){
			continue;
		}
		printw(" [%c] %s\n", selected == count ? 'x' : ' ', file->d_name);
		count++;
	}
	rewinddir(dir_buffer[depth].d_file);
	return count;
}


void swap_entries(int a, int b){
	unsigned long x_ptr = (unsigned long)dir_buffer[a].d_file ^ (unsigned long)dir_buffer[b].d_file;
	dir_buffer[a].d_file = (DIR*)((unsigned long)dir_buffer[a].d_file ^ x_ptr);
	dir_buffer[b].d_file = (DIR*)((unsigned long)dir_buffer[b].d_file ^ x_ptr);
	
	char swap_buffer[PATH_MAX];
	size_t swap_len;
	strcpy(swap_buffer, dir_buffer[a].path);
	swap_len = strlen(swap_buffer);
	strcpy(dir_buffer[a].path, dir_buffer[b].path);
	dir_buffer[a].path[strlen(dir_buffer[b].path)] = 0;
	strcpy(dir_buffer[b].path, dir_buffer[a].path);
	dir_buffer[b].path[swap_len] = 0;
}


void get_hover(char* hover){
	struct dirent *file;
	int count = 0;
	while((file = readdir(dir_buffer[depth].d_file)) != NULL){
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
	rewinddir(dir_buffer[depth].d_file);
	strncpy(hover, file->d_name, FILE_MAX);
	hover[FILE_MAX-1] = 0;
}


int is_adjacent(){
	if(depth > 0 && !strcmp(dir_buffer[depth-1].path, path)){
		depth--;
		return 1;
	}
	if(depth < DIR_MAX-1 && !strcmp(dir_buffer[depth+1].path, path)){
		depth++;
		return 1;
	}
	return 0;
}


void open_path(){
	if(depth >= DIR_MAX){
		for(int i = 0; i < 5; i++){
			closedir(dir_buffer[i].d_file);
		}
		memmove(dir_buffer, dir_buffer+5, (DIR_MAX-5)*sizeof(*dir_buffer));
		depth-=5;
		dir_count-=5;
	}
	if((depth > 0 && is_adjacent())){
		return;
	}
	while(depth != dir_count-1){
		closedir(dir_buffer[dir_count-1].d_file);
		dir_count--;
	}
	dir_buffer[dir_count].d_file = opendir(path);
	if(!dir_buffer[dir_count].d_file){
		path[path_next] = 0;
		return;
	}
	selected = 0;
	depth++;
	dir_count++;
	path_next = strlen(path);
	if(path[path_next-1] != '/'){
		path[path_next++] = '/';
		path[path_next] = 0;
	}
	strcpy(dir_buffer[depth].path, path);
}


int open_backwards(){
	char* prev;
	path[path_next-1] = 0;	
	prev = strrchr(path, '/');
	if(!prev){
		path[path_next] = '/';
		return 1;
	}
	if(prev == path){
		prev++;
	}
	*prev = 0;
	open_path();
	return 0;

}


//tries to open str and update path
void try_open(char* str, size_t str_len){
	if(str_len + path_next + 1 < PATH_MAX){
		strcat(path, str);
	}
	open_path();
}


// open hovering directory
void open_hover(){
	char hover[FILE_MAX];
	size_t hover_len;
	get_hover(hover);
	hover_len = strlen(hover);
	
	try_open(hover, hover_len);
}


// checks if a is inside b
int is_inside(char* a, char* b){
	size_t b_len = strlen(b);
	return strlen(a) > b_len && !strncmp(a, b, b_len); 	
}


// go back one directory, tries to exhaust buffer first
void move_backwards(){
	if(depth > 0 && is_inside(dir_buffer[depth].path, dir_buffer[depth-1].path)){
	// more than one file opened and current is inside previous in buffer
		selected = 0;
		strcpy(path, dir_buffer[--depth].path);
	} else{
	// one file opened or current is not inside previous in buffer
		open_backwards();
		swap_entries(depth, depth-1);
		depth--;
	}

}


int update(int direction, int max){
	int fc;
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
			return max;
	}
	clear();
	fc = ls();
	mvprintw(LINES, 0, path);
	return fc;
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
		file_count = update(action, file_count);
		refresh();
	}

	endwin();

	while(--depth >= 0){
		closedir(dir_buffer[depth].d_file);
	}
	return 0;
}
