#ifndef FAYE_NAVIGATION_H
#define FAYE_NAVIGATION_H

#include <dirent.h>
#include "cmd.h"


struct directory{
	DIR* d_file;
	char path[FAYE_PATH_MAX];
};


struct cache{
	struct directory dir_buffer[FAYE_MAX];
	int depth; // current position
	int next; // first spot available in buffer 
};


struct navigator{
	char cwd[FAYE_PATH_MAX];
	int cwd_len;
	int selected;
	int show_hidden;
};


extern struct navigator jet;
extern struct cache ein;

int ls();
int update(int, int);
void open_path();
void filter_input(char*, char*);
void initialize_cache(struct cache*);
void initialize_navigator(struct navigator*);
#endif

