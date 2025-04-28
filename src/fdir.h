#ifndef FDIR_H
#define FDIR_H

#include <dirent.h>

#define FDIR_PATH_MAX 4096
#define FDIR_FILE_MAX 255
#define FDIR_MAX 20
#define FDIR_COLS 30
#define FDIR_LINES 20


struct directory{
	DIR* d_file;
	char path[FDIR_PATH_MAX];
};


extern char path[FDIR_PATH_MAX];
extern int path_next;
extern struct directory dir_buffer[FDIR_MAX];
extern int depth; 

int ls();
int update(int, int);
void open_path();
void filter_input(char*, char*);
#endif
