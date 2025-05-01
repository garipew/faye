build: src/faye.c bin/fdir.o
	gcc -o bin/faye src/faye.c bin/fdir.o -lncurses

bin/fdir.o: src/fdir.c src/fdir.h
	gcc -o bin/fdir.o -c src/fdir.c -lncurses

clean:
	rm -rf bin/*.o bin/faye
