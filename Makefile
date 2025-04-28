faye: main.c
	gcc -o faye main.c -lncurses

clean:
	rm -rf *.o faye
