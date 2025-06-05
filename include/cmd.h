#ifndef FAYE_CMD_H
#define FAYE_CMD_H

#include "cache.h"
#include "screen.h"

struct cmd{
	char* buffer;
	int buffer_size;
	int buffer_len;
};

extern struct cmd ed;

int initialize_cmd(struct cmd*);
void read_cmd(char*);
int execute_cmd();
void free_cmd(struct cmd*);
#endif
