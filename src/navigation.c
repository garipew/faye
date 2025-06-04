#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "screen.h"
#include "navigation.h"
#include "cache.h"


struct navigator jet;


void initialize_navigator(struct navigator* n, char* path){
	memset(n->cwd, 0, FAYE_PATH_MAX);
	if(path){
		chdir(path);
	}
	getcwd(n->cwd, FAYE_PATH_MAX);
	n->cwd_len = strlen(n->cwd);
	if(n->cwd_len < FAYE_PATH_MAX && n->cwd[n->cwd_len-1] != '/'){
		n->cwd[n->cwd_len] = '/';
		n->cwd_len++;
	}
	n->selected = 0;
}


char* get_parent(){
	char* prev;
	jet.cwd[--jet.cwd_len] = 0;
	prev = strrchr(jet.cwd, '/');	
	jet.cwd[jet.cwd_len++] = '/';
	if(!prev){
		return NULL;
	}
	prev++;	
	return strndup(jet.cwd, prev-jet.cwd);
}


char* get_hover(){
	int selected = 0;
	int absolute = 0;
	while(selected <= jet.selected){
		if(!julia.show_hidden && ein.filenames[absolute][0] == '.'){
			absolute++;
			continue; // skip hidden
		}
		selected++;
		absolute++;
	}
	return ein.filenames[absolute-1];
}


int check_cache(){
	for(int i = 0; i < ein.next; i++){
		if(!strcmp(jet.cwd, ein.dir_buffer[i].path)){
			ein.dir_buffer[ein.depth].selected = jet.selected;
			ein.depth = i;
			// path found on cache, update jet.cwd_len
			jet.cwd_len = strlen(jet.cwd);
			jet.selected = ein.dir_buffer[ein.depth].selected;
			return 1;
		}
	}
	return 0;
}


int open_path(){
	if(ein.next >= FAYE_MAX){
		print_err("faye: Too many open tabs\n");
		memset(jet.cwd+jet.cwd_len, 0, FAYE_PATH_MAX-jet.cwd_len);
		return -1;
	}
	ein.dir_buffer[ein.next].d_file = opendir(jet.cwd);	
	if(!ein.dir_buffer[ein.next].d_file){
		print_err("faye: Unable to open %s\n", jet.cwd);
		memset(jet.cwd+jet.cwd_len, 0, FAYE_PATH_MAX-jet.cwd_len);
		return -2;
	}	
	jet.cwd_len = strlen(jet.cwd); // opened path, update jet.cwd_len

	ein.dir_buffer[ein.depth].selected = jet.selected;
	ein.depth = ein.next++;
	strcpy(ein.dir_buffer[ein.depth].path, jet.cwd);
	return 0;
}


void lazy_open(){
	if(check_cache() || !open_path()){
		load_files();
		julia.update = 1;
		ein.bookmark_count = 0;
	} 
}


int fix_cursor(){
	int fc = count_printable(julia.show_hidden);
	if(jet.selected >= fc){
		if(jet.selected - julia.first < fc){
			jet.selected -= julia.first;
		} else{
			jet.selected = fc-1;
		}
	}
	if(julia.first > jet.selected || julia.first+julia.max-1 < jet.selected){
		julia.first = jet.selected-julia.max+1;
		if(julia.first < 0){
			julia.first = 0;
		}
	}
	return fc;
}


void move_cursor_up(){
	if(jet.selected > 0){
		if(jet.selected == julia.first){
			julia.first--;
			julia.update = 1;
		}
		jet.selected--;
	}
}


void move_cursor_down(int max){
	if(jet.selected+1 < max){
		if(jet.selected == (julia.first + julia.max)-1){
			julia.first++;
			julia.update = 1;
		}
		jet.selected++;
	}
}


void minimize_path(){
	if(strstr(jet.cwd, "..") ||
	 strstr(jet.cwd, "./") ||
	 strrchr(jet.cwd, '.') == strchr(jet.cwd, '\0') - 1){
		chdir(jet.cwd);
		getcwd(jet.cwd, FAYE_PATH_MAX);
	}
}


void jump_to_parent(){
	char* parent = get_parent();	
	if(parent){
		strcpy(jet.cwd, parent);
		free(parent);
		lazy_open();
	}
}


void jump_to_child(){
	char* hover = get_hover();
	if(jet.cwd_len + strlen(hover) + 1 < FAYE_PATH_MAX){
		strcat(jet.cwd, hover);
		strcat(jet.cwd, "/");
		lazy_open();
	}
}


void jump_to_path(){
	read_cmd();
	if(ed.buffer[0] == '/'){
		strncpy(jet.cwd, ed.buffer, FAYE_PATH_MAX);
	} else{
		strncat(jet.cwd, ed.buffer, FAYE_PATH_MAX-strlen(jet.cwd));
	}
	minimize_path();
	if(jet.cwd[strlen(jet.cwd)-1] != '/'){
		strcat(jet.cwd, "/");
	}
	lazy_open();
}


int update(int direction, int max){
	int fc = -1;
	int pid;
	int c;
	int abs;
	switch(direction){
		case 'F':
			// free all bookmarks
			ein.bookmark_count = 0;
			break;
		case 'f':
			// free last bookmark
			if(ein.bookmark_count > 0){
				ein.bookmark_count--;
			}
			break;
		case 'b':
			// bookmark file
			abs = get_absolute(jet.selected, julia.show_hidden);
			if(is_bookmarked(abs)){
				break;
			}
			if(ein.bookmark_count < FAYE_BOOKMARKS){
				ein.bookmarks[ein.bookmark_count++] = abs;
			}else{
				ein.bookmarks[0] = abs;
			}
			break;
		case '/':
			jump_to_path();
			break;
		case ':':
			read_cmd();
			if(ed.buffer[0] == '!'){
				endwin();
				pid = fork();
				if(pid == 0){
					chdir(jet.cwd);
					dup2(fileno(faye_out), fileno(stdout));
					execute_cmd();		
					print_err("faye: Failed to execute %s\n", ed.buffer);
					exit(-2);
				} else if(pid > 0){
					waitpid(pid, NULL, 0);
					ein.bookmark_count = 0;
					print_err("\n\nfaye: Press enter to return\n\n");
					while((c = getchar()) != '\n' && c != EOF);
				} else{
					print_err("faye: Unable to fork\n");
				}
				refresh();
			}
			break;
		case 'j':
			move_cursor_down(max);
			break;
		case 'k':
			move_cursor_up();
			break;
		case 'l':
			jump_to_child();
			break;
		case 'h':
			jump_to_parent();
			break;
		case 's':
			julia.show_hidden = !julia.show_hidden;
			julia.update = 1;
			ein.bookmark_count = 0;
			break;
		default:
			return max;
	}
	fc = fix_cursor();
	return fc >= 0 ? fc : max;
}


void draw(){
	redraw(5, 2);
	clear_lines(1, 0, 0);
	mvprintw(0, 0, jet.cwd);
	clear_lines(2, 0, LINES-2);
	move((jet.selected-julia.first)+2, 0);
	refresh();
}
