CC          	= gcc
CFLAGS      	= -Wall -Wextra -Werror -I./include -D_POSIX_C_SOURCE=200809L -pthread -g
LDFLAGS     	= -pthread -lrt
NCURSES     	= -lncurses
SRC_DIR     	= src
OBJ_DIR     	= obj
BIN_DIR     	= bin
VALGRINDFLAGS 	= --leak-check=full --track-origins=yes --show-leak-kinds=all
HELGRINDFLAGS 	= --tool=helgrind
RUN_ARGS      	=
PROG        	?= server

CLIENT_OBJS 	= $(OBJ_DIR)/client.o $(OBJ_DIR)/client_ui.o $(OBJ_DIR)/transport_mq.o
SERVER_OBJS 	= $(OBJ_DIR)/server.o $(OBJ_DIR)/transport_mq.o
BINS        	= $(BIN_DIR)/client $(BIN_DIR)/server

.SILENT:

all: $(BINS)

$(BIN_DIR)/client: $(CLIENT_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CLIENT_OBJS) $(LDFLAGS) $(NCURSES) -o $@

$(BIN_DIR)/server: $(SERVER_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(SERVER_OBJS) $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN_DIR)/$(PROG)
	$(TIME) $(TIME_FLAGS) ./$(BIN_DIR)/$(PROG) $(RUN_ARGS)

valgrind: $(BIN_DIR)/$(PROG)
	$(TIME) valgrind $(VALGRINDFLAGS) ./$(BIN_DIR)/$(PROG) $(RUN_ARGS)

helgrind: $(BIN_DIR)/$(PROG)
	$(TIME) valgrind $(HELGRINDFLAGS) ./$(BIN_DIR)/$(PROG) $(RUN_ARGS)

client:
	$(MAKE) run PROG=client

server:
	$(MAKE) run PROG=server

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean run valgrind helgrind client server
