CXX = gcc
CXXFLAGS = -Wall -I./include -g
VALGRINDFLAGS = --leak-check=full --track-origins=yes

SRC_DIR = src
OBJ_DIR = obj

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))
EXECUTABLE = runner

all: valgrind

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(EXECUTABLE)
	./$(EXECUTABLE)
	@echo "Программа '$(EXECUTABLE)' завершена."

valgrind: $(EXECUTABLE)
	valgrind $(VALGRINDFLAGS) ./$(EXECUTABLE)

clean:
	rm -rf $(OBJ_DIR) $(EXECUTABLE)

.PHONY: all clean run valgrind
