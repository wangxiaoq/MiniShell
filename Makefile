minish: minish.o util.o

minish.o: minish.c
	gcc -g -c minish.c

util.o:
	gcc -g -c util.c

.PHONY: clean
clean:
	rm *.o minish
