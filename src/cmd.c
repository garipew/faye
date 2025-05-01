#include "cmd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>


struct cmd ed;


int initialize_cmd(struct cmd* b){
	b->buffer_size = 100;
	b->buffer_len = 0;
	b->buffer = malloc(b->buffer_size);
	if(!b->buffer){
		fprintf(stderr, "faye: Unable to allocate cmd buffer\n");
		return 0;
	}
	return 1;
}


void read_cmd(){
	char *new_buffer;
	int character;

	memset(ed.buffer, 0, ed.buffer_len);
	ed.buffer_len = 0;
	nocbreak();
	echo();
	move(FAYE_LINES+1, 0);
	addch(':');
	while((character=getch()) != 0 && character != '\n'){
		refresh();
		if(ed.buffer_len >= ed.buffer_size){
			ed.buffer_size*=2;
			new_buffer = realloc(ed.buffer, ed.buffer_size);
			if(!new_buffer){
				fprintf(stderr, "faye: Unable to expand cmd buffer\n");
				break;
			}
			ed.buffer = new_buffer;
		}		
		ed.buffer[ed.buffer_len++] = character;	
	}
	if(ed.buffer[ed.buffer_len-1] == '\n'){
		ed.buffer[ed.buffer_len-1] = 0;
	}
	cbreak();
	noecho();
	move(0, 0);
}


int strcnt(char* haystack, char needle){
	int c = 0;
	while(*haystack){
		if(*haystack == needle){
			c++;
		}
		haystack++;
	}
	return c;
}


int execute_cmd(){
	int argc;
	if(ed.bookmark[0]){
		argc = 2;
	}else{
		argc = strcnt(ed.buffer, ' ')+1;
	}
	char** argv;
	argv = malloc(sizeof(*argv)*(1+argc));
	if(!argv){
		return -1;
	}
	argv[argc] = NULL;
	if(ed.bookmark[0]){
		argv[0] = ed.buffer+1;
		argv[1] = ed.bookmark;
	} else{
		while(argc--){
			argv[argc] = strrchr(ed.buffer, ' ');
			if(!argv[argc]){
				argv[argc] = ed.buffer+1; // exclude the initial !
				break;
			}
			argv[argc][0] = 0;
			argv[argc]++;
		}
	}
	execvp(argv[0], argv);
	free(argv);
	return -2;	
}
