minish: minish.o

minish.o: minish.c
	gcc -g -c minish.c

.PHONY: clean
clean:
	rm *.o minish
