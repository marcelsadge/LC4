trace: LC4.c loader.c trace.c loader.h LC4.h
	clang -Wall -o trace LC4.c loader.c trace.c
clean:
	rm trace *.o