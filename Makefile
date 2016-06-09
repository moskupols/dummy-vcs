CFLAGS := -std=c99
CFLAGS += -Wall -Wextra -Wformat=2 -Wno-char-subscripts
CFLAGS += -O0 -ggdb
CFLAGS += -fsanitize=address -fsanitize=undefined -lmcheck

SOURCES := $(wildcard *.c)
HEADERS := $(wildcard *.h)
OBJECTS := $(SOURCES:.c=.o)

TARGET := main

main : $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

debug : $(TARGET)
	gdb -q -tui $<

gdb : debug

clean :
	for i in $(wildcard *.o main a.*); do rm $$i; done

.PHONY : debug gdb clean

