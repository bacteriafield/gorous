CFLAGS = -Wall -Wextra -Iinclude -O2

gorous: gorous.o elf.o
gorous.o elf.o: include/elf.h

clean:
	rm -f gorous *.o

.PHONY: clean
