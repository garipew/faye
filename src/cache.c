#include "cache.h"
#include "screen.h"
#include <stdlib.h>
#include <string.h>


struct cache ein;

int initialize_cache(struct cache* c){
	memset(c->dir_buffer, 0, sizeof(c->dir_buffer[0]) * FAYE_MAX);
	c->depth = -1;
	c->next = 0;
	
	c->file_slots = 10; 
	c->files = 0;
	c->filenames = malloc(sizeof(*c->filenames)*c->file_slots);
	if(!c->filenames){
		print_err("faye: Unable to initialize cache\n");
		return -1;
	}
	memset(c->filenames, 0, c->file_slots*sizeof(*c->filenames));
	return 0;
}


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
				print_err("faye: Unable to expand cache.\n");
				ein.file_slots/=2; 
				break;
			}	
			ein.filenames = new_buffer;
			memset(ein.filenames+ein.files, 0, sizeof(*ein.filenames)*(ein.file_slots-ein.files+1));
		} 	
		if(!ein.filenames[ein.files]){
			ein.filenames[ein.files] = malloc(FAYE_FILE_MAX);
			if(!ein.filenames[ein.files]){
				print_err("faye: Unable to expand cache.\n");
				break;
			}
		}
		strcpy(ein.filenames[ein.files], file->d_name);
		ein.files++;	
	}
	rewinddir(ein.dir_buffer[ein.depth].d_file);
	return ein.files;
}


int count_printable(int show_hidden){
	int printable = 0;
	for(int i = 0; i < ein.files; i++){
		if(!show_hidden && ein.filenames[i][0] == '.'){
			continue;
		}
		printable++;
	}
	return printable;
}


int is_bookmarked(int needle){
	for(int i = 0; i < ein.bookmark_count; i++){
		if(needle == ein.bookmarks[i]){
			return 1;
		}
	}
	return 0;
}


int get_absolute(int relative, int show_hidden){
	int absolute;
	int printable = 0;
	int max = count_printable(1);
	for(absolute = 0; absolute < max; absolute++){
		if(!show_hidden && ein.filenames[absolute][0] == '.'){
			continue;
		}	
		if(printable == relative){
			break;
		}
		printable++;
	}
	return absolute;
}


int get_relative(int absolute, int show_hidden){
	int count;
	int printable = 0;
	int max = count_printable(1);
	for(count = 0; count < absolute; count++){
		if(!show_hidden && ein.filenames[count][0] == '.'){
			continue;
		}	
		printable++;
	}
	return printable;
}
