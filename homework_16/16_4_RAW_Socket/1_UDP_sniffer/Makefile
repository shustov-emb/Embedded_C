CC          	= gcc
CFLAGS      	= -Wall -Wextra -Werror -g
LDFLAGS     	=
SRC_DIR     	= src
OBJ_DIR     	= obj
BIN_DIR     	= bin
VALGRINDFLAGS 	= --leak-check=full --track-origins=yes --show-leak-kinds=all
HELGRINDFLAGS 	= --tool=helgrind
RUN_ARGS      	=
SRCS        	= $(wildcard $(SRC_DIR)/*.c)
BINS        	= $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%, $(SRCS))
PROG        	?= client

.SILENT:

all: $(BINS)

$(BIN_DIR)/%: $(OBJ_DIR)/%.o
	@mkdir -p $(BIN_DIR)
	$(CC) $< $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN_DIR)/$(PROG)
	$(TIME) $(TIME_FLAGS) ./$(BIN_DIR)/$(PROG) $(RUN_ARGS)

valgrind: $(BIN_DIR)/$(PROG)
	$(TIME) valgrind $(VALGRINDFLAGS) ./$(BIN_DIR)/$(PROG)

helgrind: $(BIN_DIR)/$(PROG)
	$(TIME) valgrind $(HELGRINDFLAGS) ./$(BIN_DIR)/$(PROG)

client:
	$(MAKE) run PROG=client

server:
	$(MAKE) run PROG=server

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean run valgrind helgrind client server
