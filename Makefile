minish: minish.o util.o key-handler.o history-cmd.o signal-handler.o environment.o complete.o print.o

COMFLAGS = -g -c
CC = gcc

minish.o: minish.c
	$(CC) $(COMFLAGS) $<

util.o: util.c util.h
	$(CC) $(COMFLAGS) $<

key-handler.o: key-handler.c key-handler.h
	$(CC) $(COMFLAGS) $<

history-cmd.o: history-cmd.c history-cmd.h
	$(CC) $(COMFLAGS) $<

signal-handler.o: signal-handler.c signal-handler.h
	$(CC) $(COMFLAGS) $<

environment.o: environment.c environment.h
	$(CC) $(COMFLAGS) $<

complete.o: complete.c complete.h
	$(CC) $(COMFLAGS) $<

print.o: print.c print.h
	$(CC) $(COMFLAGS) $<

.PHONY: clean
clean:
	rm *.o minish
