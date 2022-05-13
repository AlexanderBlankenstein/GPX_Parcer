UNAME := $(shell uname)
CC = gcc
CFLAGS = -Wall -std=c11 -g
LDFLAGS= -L.

INC = include/
SRC = src/
BIN = bin/
PARSER_SRC_FILES = $(wildcard src/GPX*.c)
PARSER_OBJ_FILES = $(patsubst src/GPX%.c,bin/GPX%.o,$(PARSER_SRC_FILES))

ifeq ($(UNAME), Linux)
	XML_PATH = /usr/include/libxml2
endif
ifeq ($(UNAME), Darwin)
	XML_PATH = /System/Volumes/Data/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/libxml2
endif

parser: $(BIN)libgpxparser.so

$(BIN)libgpxparser.so: $(PARSER_OBJ_FILES) $(BIN)LinkedListAPI.o
	gcc -shared -o $(BIN)libgpxparser.so $(PARSER_OBJ_FILES) $(BIN)LinkedListAPI.o -lxml2 -lm

#Compiles all files named GPX*.c in src/ into object files, places all coresponding GPX*.o files in bin/
$(BIN)GPX%.o: $(SRC)GPX%.c $(INC)LinkedListAPI.h $(INC)GPX*.h
	gcc $(CFLAGS) -I$(XML_PATH) -I$(INC) -c -fpic $< -o $@

$(BIN)liblist.so: $(BIN)LinkedListAPI.o
	$(CC) -shared -o $(BIN)liblist.so $(BIN)LinkedListAPI.o

$(BIN)LinkedListAPI.o: $(SRC)LinkedListAPI.c $(INC)LinkedListAPI.h
	$(CC) $(CFLAGS) -c -fpic -I$(INC) $(SRC)LinkedListAPI.c -o $(BIN)LinkedListAPI.o

clean:
	rm -rf $(BIN)*.o $(BIN)*.so

###################################################################################################
