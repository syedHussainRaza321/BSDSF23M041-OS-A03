

CC       = gcc
CFLAGS   = -Wall -Wextra -g -Iinclude
LIBS     = -lreadline
SRC      = src/main.c src/shell.c src/execute.c
OBJ      = obj/main.o obj/shell.o obj/execute.o
BIN      = bin/myshell

all: $(BIN)

$(BIN): $(OBJ)
	@mkdir -p bin
	$(CC) $(OBJ) -o $(BIN) $(LIBS)

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj/*.o $(BIN)

.PHONY: all clean
