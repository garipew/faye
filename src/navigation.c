#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "screen.h"
#include "navigation.h"


struct navigator jet;
struct cache ein;


void free_cache(struct cache* c){
	if(c->filenames){
		for(int i = 0; i < c->files; i++){
			free(c->filenames[i]);
		}
		free(c->filenames);
	}
	for(int i = 0; i < c->next; i++){
		closedir(c->dir_buffer[i].d_file);
	}
}


void initialize_navigator(struct navigator* n, char* path){
	memset(n->cwd, 0, FAYE_PATH_MAX);
	if(!path){
		getcwd(n->cwd, FAYE_PATH_MAX);
	}else{
		strncpy(n->cwd, path, FAYE_PATH_MAX);
	}
	n->cwd_len = strlen(n->cwd);
	if(n->cwd_len < FAYE_PATH_MAX && n->cwd[n->cwd_len-1] != '/'){
		n->cwd[n->cwd_len] = '/';
		n->cwd_len++;
	}
	n->selected = 0;
	n->show_hidden = 0;
}


int initialize_cache(struct cache* c){
	memset(c->dir_buffer, 0, sizeof(c->dir_buffer[0]) * FAYE_MAX);
	c->depth = -1;
	c->next = 0;
	
	c->file_slots = 10; 
	c->files = 0;
	c->filenames = malloc(sizeof(*c->filenames)*c->file_slots);
	if(!c->filenames){
		fprintf(stderr, "faye: Unable to initialize cache\n");
		return -1;
	}
	memset(c->filenames, 0, c->file_slots*sizeof(*c->filenames));
	return 0;
}


int load_files(){
	struct dirent *file;
	char** new_buffer;
	ein.files = 0;
	while((file = readdir(ein.dir_buffer[ein.depth].d_file)) != NULL){
		if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, "..")){
			continue;
		}
		if(ein.files >= ein.file_slots){
			ein.file_slots*=2; 
			new_buffer = realloc(ein.filenames, sizeof(*ein.filenames)*ein.file_slots);
			if(!new_buffer){
				fprintf(stderr, "faye: Unable to expand cache.\n");
				ein.file_slots/=2; 
				break;
			}	
			ein.filenames = new_buffer;
			memset(ein.filenames+ein.files, 0, sizeof(*ein.filenames)*(ein.file_slots-ein.files+1));
		} 	
		if(!ein.filenames[ein.files]){
			ein.filenames[ein.files] = malloc(FAYE_FILE_MAX);
			if(!ein.filenames[ein.files]){
				fprintf(stderr, "faye: Unable to expand cache.\n");
				break;
			}
		}
		strcpy(ein.filenames[ein.files], file->d_name);
		ein.files++;	
	}
	rewinddir(ein.dir_buffer[ein.depth].d_file);
	return ein.files;
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


char* get_hover(){
	int selected = 0;
	int absolute = 0;
	while(selected <= jet.selected){
		fprintf(stderr, "Not %s\n", ein.filenames[absolute]);
		if(!jet.show_hidden && ein.filenames[absolute][0] == '.'){
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
			ein.depth = i;
			jet.selected = 0;
			// path found on cache, update jet.cwd_len
			jet.cwd_len = strlen(jet.cwd);
			return 1;
		}
	}
	return 0;
}


int open_path(){
	if(ein.next >= FAYE_MAX){
		fprintf(stderr, "faye: Too many open tabs\n");
		return -1;
	}
	ein.dir_buffer[ein.next].d_file = opendir(jet.cwd);	
	if(!ein.dir_buffer[ein.next].d_file){
		fprintf(stderr, "faye: Unable to open %s\n", jet.cwd);
		memset(jet.cwd+jet.cwd_len, 0, FAYE_PATH_MAX-jet.cwd_len);
		return -2;
	}	
	jet.cwd_len = strlen(jet.cwd); // opened path, update jet.cwd_len
	jet.selected = 0;
	ein.depth = ein.next;
	strcpy(ein.dir_buffer[ein.next].path, jet.cwd);
	ein.next++;	
	return 0;
}


void close_dir(){
	closedir(ein.dir_buffer[ein.depth].d_file);
	ein.next--;
	memmove(&ein.dir_buffer[ein.depth], &ein.dir_buffer[ein.depth+1], (ein.next - ein.depth)*sizeof(ein.dir_buffer[ein.depth]));
	if(ein.depth == ein.next){
		ein.depth--;
	}
}


int print_files(int x, int y){
	if(ein.files == 0){
		return 0;
	}
	int line = 0;
	for(int i = 0; i < ein.files; i++){
		if(!jet.show_hidden && ein.filenames[i][0] == '.'){
			continue;
		}
		mvprintw(y+line, x, ein.filenames[i]);	
		line++;
	}
	return line;
}


int update(int direction, int max){
	int fc = -1;
	int pid;
	int c;
	char* hover;
	switch(direction){
		case 'f':
			// free ed.bookmark
			ed.bookmark[0] = 0;
			break;
		case 'b':
			// ed.bookmark file
			strcpy(ed.bookmark, jet.cwd);
			strcat(ed.bookmark, get_hover());
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
				load_files();
			}
			break;
		case 'K':
			if(ein.depth > 0){
				ein.depth--;
				jet.selected = 0;
				strcpy(jet.cwd, ein.dir_buffer[ein.depth].path);
				jet.cwd_len = strlen(jet.cwd);
				load_files();
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
			strcat(jet.cwd, get_hover());
			strcat(jet.cwd, "/");
			if(check_cache() || !open_path()){
				load_files();
			}
			break;
		case 'h':
			if(get_return() && (check_cache() || !open_path())){
				load_files();
			}
			break;
		case 's':
			jet.show_hidden = !jet.show_hidden;
			break;
		default:
			return max;
	}
	clear();
	mvprintw(0, 0, jet.cwd);
	fc = print_files(4, 1);
	if(jet.selected >= fc){
		jet.selected = fc -1;
	}
	mvprintw(FAYE_LINES-2, 0, "[*] %s", ed.bookmark);
	move(jet.selected+1, 0);
	refresh();
	return fc >= 0 ? fc : max;
}
