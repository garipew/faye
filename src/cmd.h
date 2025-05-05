#ifndef FAYE_CMD_H
#define FAYE_CMD_H

#include "screen.h"

struct cmd{
	char* buffer;
	int buffer_size;
	int buffer_len;
	char bookmark[FAYE_PATH_MAX];
};

extern struct cmd ed;

int initialize_cmd(struct cmd*);
void read_cmd();
int execute_cmd();
void free_cmd(struct cmd*);
#endif
