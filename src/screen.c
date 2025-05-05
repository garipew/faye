#include "screen.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>


int faye_fdout;
FILE* faye_out;
SCREEN* faye_screen;
struct content julia;


void initialize_content(struct content* c){
	c->first = 0;
	c->max = 15;
}


int set_output(){
	faye_fdout = fileno(stdout);	
	faye_out = stdout;
	if(!isatty(faye_fdout)){
		faye_fdout = open("/dev/tty", O_WRONLY);
		if(faye_fdout < 0){
			return -1;
		}
		faye_out = fdopen(faye_fdout, "w");
		if(!faye_out){
			close(faye_fdout);
			return -2;
		}
	}
	return 0;
}


int start_screen(){
	if(set_output() < 0){
		return -1;
	}
	faye_screen = newterm(NULL, faye_out, stdin);
	return 0;
}


void unset_output(){
	int fd_out = fileno(stdout);
	if(faye_fdout != fd_out){
		close(faye_fdout);
		faye_fdout = fd_out;
		fclose(faye_out);
		faye_out = stdout;
	}
}
