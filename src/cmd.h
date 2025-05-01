#ifndef FAYE_CMD_H
#define FAYE_CMD_H

#define FAYE_PATH_MAX 4096
#define FAYE_FILE_MAX 255
#define FAYE_MAX 20
#define FAYE_COLS 30
#define FAYE_LINES 20


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
#endif
