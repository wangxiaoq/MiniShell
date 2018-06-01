minish: minish.o util.o key-handler.o history-cmd.o signal-handler.o environment.o

minish.o: minish.c
	gcc -g -c minish.c

util.o: util.c util.h
	gcc -g -c util.c

key-handler.o: key-handler.c key-handler.h
	gcc -g -c key-handler.c

history-cmd.o: history-cmd.c history-cmd.h
	gcc -g -c history-cmd.c

signal-handler.o: signal-handler.c signal-handler.h
	gcc -g -c signal-handler.c

environment.o: environment.c environment.h
	gcc -g -c environment.c

.PHONY: clean
clean:
	rm *.o minish
