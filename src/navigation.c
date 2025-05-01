#include "navigation.h"
#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


struct navigator jet;
struct cache ein;


void initialize_navigator(struct navigator* n){
	memset(n->cwd, 0, FAYE_PATH_MAX);
	n->cwd_len = 0;
	n->selected = 0;
	n->show_hidden = 0;
}


void initialize_cache(struct cache* c){
	memset(c->dir_buffer, 0, sizeof(c->dir_buffer[0]) * FAYE_MAX);
	c->depth = -1;
	c->next = 0;
}


int ls(){
	struct dirent *file;
	int count = 0;
	while((file = readdir(ein.dir_buffer[ein.depth].d_file)) != NULL){
		if(!jet.show_hidden && file->d_name[0] == '.'){
			continue;
		}
		if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, "..")){
			continue;
		}
		printw(" [%c] %s\n", jet.selected == count ? 'x' : ' ', file->d_name);
		count++;
	}
	rewinddir(ein.dir_buffer[ein.depth].d_file);
	mvprintw(FAYE_LINES+2, 0, ed.bookmark);
	return count;
}


int get_return(){
	char* prev;
	jet.cwd[--jet.cwd_len] = 0;
	prev = strrchr(jet.cwd, '/');	
	if(!prev){
		jet.cwd[jet.cwd_len++] = '/';
		return 0;
	}
	prev++;	
	memset(prev, 0, jet.cwd_len + jet.cwd - prev);	
	return 1;
}


int get_hover(){
	struct dirent *file;
	int count = 0;
	char hover[FAYE_FILE_MAX] = {0};
	int hover_len;
	while((file = readdir(ein.dir_buffer[ein.depth].d_file)) != NULL){
		if(!jet.show_hidden && file->d_name[0] == '.'){
			continue;
		}
		if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, "..")){
			continue;
		}
		if(count == jet.selected){
			break;
		}
		count++;
	}
	if(!file){
		return 0;
	}
	rewinddir(ein.dir_buffer[ein.depth].d_file);
	strncpy(hover, file->d_name, FAYE_FILE_MAX);
	hover_len = strnlen(hover, FAYE_FILE_MAX);
	if(hover_len < FAYE_FILE_MAX && hover[hover_len-1] != '/'){
		hover[hover_len] = '/';
	}
	strncpy(jet.cwd+jet.cwd_len, hover, PATH_MAX-jet.cwd_len);
	return 1;
}


int check_cache(){
	for(int i = 0; i < ein.next; i++){
		if(!strcmp(jet.cwd, ein.dir_buffer[i].path)){
			ein.depth = i;
			jet.selected = 0;
			// path found on cache, update jet.cwd_len
			jet.cwd_len = strlen(jet.cwd);
			return 1;
		}
	}
	return 0;
}


void open_path(){
	if(ein.next >= FAYE_MAX){
		// buffer full
		return;
	}
	ein.dir_buffer[ein.next].d_file = opendir(jet.cwd);	
	if(!ein.dir_buffer[ein.next].d_file){
		// unable to open path, drop filename
		memset(jet.cwd+jet.cwd_len, 0, FAYE_PATH_MAX-jet.cwd_len);
		return;
	}	
	jet.cwd_len = strlen(jet.cwd); // opened path, update jet.cwd_len
	jet.selected = 0;
	ein.depth = ein.next;
	strcpy(ein.dir_buffer[ein.next].path, jet.cwd);
	ein.next++;	
}


void close_dir(){
	closedir(ein.dir_buffer[ein.depth].d_file);
	ein.next--;
	memmove(&ein.dir_buffer[ein.depth], &ein.dir_buffer[ein.depth+1], (ein.next - ein.depth)*sizeof(ein.dir_buffer[ein.depth]));
	if(ein.depth == ein.next){
		ein.depth--;
	}
}


int update(int direction, int max){
	int fc;
	int pid;
	int c;
	switch(direction){
		case 'f':
			// free ed.bookmark
			ed.bookmark[0] = 0;
			break;
		case 'b':
			// ed.bookmark file
			if(get_hover()){
				strcpy(ed.bookmark, jet.cwd);
				ed.bookmark[strlen(ed.bookmark)-1] = 0;
				jet.cwd[jet.cwd_len] = 0;
			}
			break;
		case ':':
			read_cmd();
			if(ed.buffer[0] == '!'){
				endwin();
				pid = fork();
				if(pid == 0){
					chdir(jet.cwd);
					execute_cmd();		
					fprintf(stderr, "faye: Failed to execute %s\n", ed.buffer);
					exit(-2);
				} else if(pid > 0){
					waitpid(pid, NULL, 0);
					ed.bookmark[0] = 0;
					fprintf(stderr, "\n\nfaye: Press enter to return\n\n");
					while((c = getchar()) != '\n' && c != EOF);
				} else{
					fprintf(stderr, "faye: Unable to fork\n");
				}
				refresh();
			}
			break;
		case 'c':
			if(ein.next > 1){
				close_dir();
				jet.selected = 0;
				strcpy(jet.cwd, ein.dir_buffer[ein.depth].path);
				jet.cwd_len = strlen(jet.cwd);
			}
			break;
		case 'J':
			if(ein.depth < ein.next-1){
				ein.depth++;
				jet.selected = 0;
				strcpy(jet.cwd, ein.dir_buffer[ein.depth].path);
				jet.cwd_len = strlen(jet.cwd);
			}
			break;
		case 'K':
			if(ein.depth > 0){
				ein.depth--;
				jet.selected = 0;
				strcpy(jet.cwd, ein.dir_buffer[ein.depth].path);
				jet.cwd_len = strlen(jet.cwd);
			}
			break;
		case 'j':
			if(jet.selected < max-1){
				jet.selected++;
			}
			break;
		case 'k':
			if(jet.selected > 0){
				jet.selected--;
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
			jet.show_hidden = !jet.show_hidden;
			break;
		default:
			return max;
	}
	clear();
	fc = ls();
	mvprintw(FAYE_LINES, 0, jet.cwd);
	for(int i = 0; i < ein.next; i++){
		mvprintw(i, FAYE_COLS, "[%c] %s\n", i == ein.depth ? 'x' : ' ', ein.dir_buffer[i].path);
	}
	return fc;
}


void filter_input(char* input, char* buffer){
	if(input[0] == '/'){
		strncpy(buffer, input, FAYE_PATH_MAX);
		buffer[FAYE_PATH_MAX-1] = 0;
	} else{
		strcat(buffer, "/");
		strcat(buffer, input);
	}
}

