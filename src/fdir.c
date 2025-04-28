#include "fdir.h"
#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


char path[FDIR_PATH_MAX];
int path_next;
int selected;
struct directory dir_buffer[FDIR_MAX];
int depth; // where current dir is on buffer
int dir_count; // where next open dir will be placed on buffer
int show_hidden;
int depth = -1;


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
	
	char swap_buffer[FDIR_PATH_MAX];
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
	strncpy(hover, file->d_name, FDIR_FILE_MAX);
	hover[FDIR_FILE_MAX-1] = 0;
}


int is_adjacent(){
	if(depth > 0 && !strcmp(dir_buffer[depth-1].path, path)){
		depth--;
		return 1;
	}
	if(depth < FDIR_MAX-1 && !strcmp(dir_buffer[depth+1].path, path)){
		depth++;
		return 1;
	}
	return 0;
}


void open_path(){
	if(depth >= FDIR_MAX){
		for(int i = 0; i < 5; i++){
			closedir(dir_buffer[i].d_file);
		}
		memmove(dir_buffer, dir_buffer+5, (FDIR_MAX-5)*sizeof(*dir_buffer));
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
	if(str_len + path_next + 1 < FDIR_PATH_MAX){
		strcat(path, str);
	}
	open_path();
}


// open hovering directory
void open_hover(){
	char hover[FDIR_FILE_MAX];
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
	mvprintw(FDIR_LINES, 0, path);
	return fc;
}


void filter_input(char* input, char* buffer){
	if(input[0] == '/'){
		strncpy(buffer, input, sizeof(path));
		buffer[FDIR_PATH_MAX-1] = 0;
	} else{
		strcat(buffer, "/");
		strcat(buffer, input);
	}
}


