build: src/faye.c bin/fdir.o
	gcc -o faye src/faye.c bin/fdir.o -lncurses
	mv faye bin

bin/fdir.o: src/fdir.c src/fdir.h
	gcc -c src/fdir.c -lncurses
	mv fdir.o bin

clean:
	rm -rf bin/*.o bin/faye
