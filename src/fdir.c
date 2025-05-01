#include "fdir.h"
#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


char path[FDIR_PATH_MAX];
int path_next;
int selected;
struct directory dir_buffer[FDIR_MAX];
int depth; // where current dir is on buffer
int dir_count; // where next open dir will be placed on buffer
int show_hidden;
int depth = -1;
struct cmd bar;


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


int get_return(){
	char* prev;
	path[--path_next] = 0;
	prev = strrchr(path, '/');	
	if(!prev){
		path[path_next++] = '/';
		return 0;
	}
	prev++;	
	memset(prev, 0, path_next + path - prev);	
	return 1;
}


int get_hover(){
	struct dirent *file;
	int count = 0;
	char hover[FDIR_FILE_MAX] = {0};
	int hover_len;
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
	if(!file){
		return 0;
	}
	rewinddir(dir_buffer[depth].d_file);
	strncpy(hover, file->d_name, FDIR_FILE_MAX);
	hover_len = strnlen(hover, FDIR_FILE_MAX);
	if(hover_len < FDIR_FILE_MAX && hover[hover_len-1] != '/'){
		hover[hover_len] = '/';
	}
	strncpy(path+path_next, hover, PATH_MAX-path_next);
	return 1;
}


int check_cache(){
	for(int i = 0; i < dir_count; i++){
		if(!strcmp(path, dir_buffer[i].path)){
			depth = i;
			selected = 0;
			// path found on cache, update path_next
			path_next = strlen(path);
			return 1;
		}
	}
	return 0;
}


void open_path(){
	if(dir_count >= FDIR_MAX){
		// buffer full
		return;
	}
	dir_buffer[dir_count].d_file = opendir(path);	
	if(!dir_buffer[dir_count].d_file){
		// unable to open path, drop filename
		memset(path+path_next, 0, FDIR_PATH_MAX-path_next);
		return;
	}	
	path_next = strlen(path); // opened path, update path_next
	selected = 0;
	depth = dir_count;
	strcpy(dir_buffer[dir_count].path, path);
	dir_count++;	
}


void close_dir(){
	closedir(dir_buffer[depth].d_file);
	dir_count--;
	memmove(&dir_buffer[depth], &dir_buffer[depth+1], (dir_count - depth)*sizeof(dir_buffer[depth]));
	if(depth == dir_count){
		depth--;
	}
}


int initialize_cmd_bar(struct cmd* bar){
	bar->buffer_size = 100;
	bar->buffer_len = 0;
	bar->buffer = malloc(bar->buffer_size);
	if(!bar->buffer){
		fprintf(stderr, "faye: Unable to allocate cmd buffer\n");
		return 0;
	}
	return 1;
}


void read_cmd(){
	char *new_buffer;
	int character;

	memset(bar.buffer, 0, bar.buffer_len);
	bar.buffer_len = 0;
	nocbreak();
	echo();
	move(FDIR_LINES+1, 0);
	addch(':');
	while((character=getch()) != 0 && character != '\n'){
		refresh();
		if(bar.buffer_len >= bar.buffer_size){
			bar.buffer_size*=2;
			new_buffer = realloc(bar.buffer, bar.buffer_size);
			if(!new_buffer){
				fprintf(stderr, "faye: Unable to expand cmd buffer\n");
				break;
			}
			bar.buffer = new_buffer;
		}		
		bar.buffer[bar.buffer_len++] = character;	
	}
	if(bar.buffer[bar.buffer_len-1] == '\n'){
		bar.buffer[bar.buffer_len-1] = 0;
	}
	cbreak();
	noecho();
	move(0, 0);
}


int strcnt(char* haystack, char needle){
	int c = 0;
	while(*haystack){
		if(*haystack == needle){
			c++;
		}
		haystack++;
	}
	return c;
}


int execute_bar(){
	int argc = strcnt(bar.buffer, ' ')+1;
	char** argv;
	if(argc < 1){
		return -1;
	}
	argv = malloc(sizeof(*argv)*(1+argc));
	if(!argv){
		return -2;
	}
	argv[argc] = NULL;
	while(argc--){
		argv[argc] = strrchr(bar.buffer, ' ');
		if(!argv[argc]){
			argv[argc] = bar.buffer+1; // exclude the initial !
			break;
		}
		argv[argc][0] = 0;
		argv[argc]++;
	}
	execvp(argv[0], argv);
	free(argv);
	return -3;	
}


int update(int direction, int max){
	int fc;
	int pid;
	switch(direction){
		case ':':
			read_cmd();
			if(bar.buffer[0] == '!'){
				endwin();
				pid = fork();
				if(pid == 0){
					chdir(path);
					execute_bar();		
					fprintf(stderr, "faye: Failed to execute %s\n", bar.buffer);
					exit(-2);
				} else if(pid > 0){
					waitpid(pid, NULL, 0);
					printf("\nPress any key to return to faye\n");
					getchar();
				} else{
					fprintf(stderr, "faye: Unable to fork\n");
				}
				refresh();
			}
			break;
		case 'c':
			if(dir_count > 1){
				close_dir();
				selected = 0;
				strcpy(path, dir_buffer[depth].path);
				path_next = strlen(path);
			}
			break;
		case 'J':
			if(depth < dir_count-1){
				depth++;
				selected = 0;
				strcpy(path, dir_buffer[depth].path);
				path_next = strlen(path);
			}
			break;
		case 'K':
			if(depth > 0){
				depth--;
				selected = 0;
				strcpy(path, dir_buffer[depth].path);
				path_next = strlen(path);
			}
			break;
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
			if(get_hover() && !check_cache()){
				open_path();
			}
			break;
		case 'h':
			if(get_return() && !check_cache()){
				open_path();
			}
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
	for(int i = 0; i < dir_count; i++){
		mvprintw(i, FDIR_COLS, "[%c] %s\n", i == depth ? 'x' : ' ', dir_buffer[i].path);
	}
	return fc;
}


void filter_input(char* input, char* buffer){
	if(input[0] == '/'){
		strncpy(buffer, input, FDIR_PATH_MAX);
		buffer[FDIR_PATH_MAX-1] = 0;
	} else{
		strcat(buffer, "/");
		strcat(buffer, input);
	}
}

