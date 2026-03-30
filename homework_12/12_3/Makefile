CC          = gcc
CFLAGS      = -g -Wall -Wextra -Werror -I./include
LDFLAGS     = 
SRC_DIR     = src
OBJ_DIR     = obj
BIN_DIR     = bin
RUN_ARGS    = 
EXECUTABLE  = $(BIN_DIR)/run
VALGRINDFLAGS = --leak-check=full --track-origins=yes --show-leak-kinds=all -s

SRCS        = $(wildcard $(SRC_DIR)/*.c)
OBJS        = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(EXECUTABLE)
	./$(EXECUTABLE) $(RUN_ARGS)

valgrind: $(EXECUTABLE)
	valgrind $(VALGRINDFLAGS) ./$(EXECUTABLE)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) output.txt

.PHONY: all clean run valgrind 