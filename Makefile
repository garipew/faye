CC = gcc
CFLAGS = -I./include

build: src/faye.c bin/cmd.o bin/navigation.o bin/screen.o bin/cache.o
	$(CC) $(CFLAGS) -o bin/faye src/faye.c bin/screen.o bin/navigation.o bin/cache.o bin/cmd.o -lncurses

bin/cmd.o: src/cmd.c include/cmd.h
	$(CC) $(CFLAGS) -o bin/cmd.o -c src/cmd.c -lncurses

bin/navigation.o: src/navigation.c include/navigation.h
	$(CC) $(CFLAGS) -o bin/navigation.o -c src/navigation.c -lncurses

bin/screen.o: src/screen.c include/screen.h
	$(CC) $(CFLAGS) -o bin/screen.o -c src/screen.c -lncurses

bin/cache.o: src/cache.c include/cache.h
	$(CC) $(CFLAGS) -o bin/cache.o -c src/cache.c -lncurses

clean:
	rm -rf bin/*.o bin/faye
