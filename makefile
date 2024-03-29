
GIT_HOOKS := .git/hooks/applied
CC := gcc
CFLAGS += -std=gnu99 -g -Wall
LIB += -ljson-c

all: $(GIT_HOOKS) 

$(GIT_HOOKS):
	@.githooks/install-git-hooks
	@echo

simulator:simulator.o os2021_thread_api.o function_libary.o cJSON.c
	$(CC) $(CFLAGS) -o simulator cJSON.c simulator.o os2021_thread_api.o function_libary.o $(LIB)

simulator.o:simulator.c os2021_thread_api.h
	$(CC) $(CFLAGS) -c simulator.c $(LIB)

os2021_thread_api.o:os2021_thread_api.c os2021_thread_api.h function_libary.h
	$(CC) $(CFLAGS) -c os2021_thread_api.c $(LIB)

function_libary.o: function_libary.c function_libary.h
	$(CC) $(CFLAGS) -c function_libary.c $(LIB)

.PHONY: clean
clean:
	rm *.o
