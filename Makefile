CC = gcc
CFLAGS = -I./src

build: src/faye.c bin/cmd.o bin/navigation.o
	$(CC) $(CFLAGS) -o bin/faye src/faye.c bin/navigation.o bin/cmd.o -lncurses

bin/cmd.o: src/cmd.c src/cmd.h
	$(CC) $(CFLAGS) -o bin/cmd.o -c src/cmd.c -lncurses

bin/navigation.o: src/navigation.c src/navigation.h
	$(CC) $(CFLAGS) -o bin/navigation.o -c src/navigation.c -lncurses

clean:
	rm -rf bin/*.o bin/faye
