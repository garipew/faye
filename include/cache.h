#ifndef FAYE_CACHE_H
#define FAYE_CACHE_H

#include <dirent.h>


#define FAYE_PATH_MAX 4096
#define FAYE_FILE_MAX 255
#define FAYE_MAX 20
#define FAYE_COLS 30
#define FAYE_LINES 20
#define FAYE_BOOKMARKS 5


struct directory{
	DIR* d_file;
	char path[FAYE_PATH_MAX];
	int selected;
};


struct cache{
	struct directory dir_buffer[FAYE_MAX];
	int depth; // current position
	int next; // first spot available in buffer 

	char** filenames;	
	int files;
	int file_slots;

	int bookmarks[FAYE_BOOKMARKS];
	int bookmark_count;
};


extern struct cache ein;

int initialize_cache(struct cache*);
void free_cache(struct cache*);
int load_files();
int count_printable(int);
int is_bookmarked(int);
#endif
