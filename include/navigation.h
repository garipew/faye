#ifndef FAYE_NAVIGATION_H
#define FAYE_NAVIGATION_H

#include <dirent.h>
#include "cache.h"
#include "cmd.h"


struct navigator{
	char cwd[FAYE_PATH_MAX];
	int cwd_len;
	int selected;
};


extern struct navigator jet;

int update(int, int);
int open_path();
void filter_input(char*, char*);
void initialize_navigator(struct navigator*, char*);
void draw();
#endif

