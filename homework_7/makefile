CXX = gcc
CXXFLAGS = -Wall -I./include -g
VALGRINDFLAGS = --leak-check=full --track-origins=yes

SRC_DIR = src
OBJ_DIR = obj
LIB_DIR = lib

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

LIB_NAME = libcalc.a #Имя библиотеки
LIB_SHORT_NAME = $(patsubst lib%,%,$(basename $(LIB_NAME))) #Короткое имя библиотеки для -l
LIB_OBJECTS = $(filter-out $(OBJ_DIR)/main.o, $(OBJECTS)) #Берём все исходные файлы кроме main.c

EXECUTABLE = runner


all: $(LIB_DIR)/$(LIB_NAME) $(EXECUTABLE)

$(EXECUTABLE): $(OBJ_DIR)/main.o $(LIB_DIR)/$(LIB_NAME)
	$(CXX) $(OBJ_DIR)/main.o -L$(LIB_DIR) -l$(LIB_SHORT_NAME) -o $@

# Создаём библиотеку в папке lib
$(LIB_DIR)/$(LIB_NAME): $(LIB_OBJECTS)
	@mkdir -p $(LIB_DIR)
	ar rc $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(EXECUTABLE)
	./$(EXECUTABLE)
	@echo "Программа '$(EXECUTABLE)' завершена."

#Нет выделения памяти, проверять нет смысла
# valgrind: $(EXECUTABLE)
# 	valgrind $(VALGRINDFLAGS) ./$(EXECUTABLE)

clean:
	rm -rf $(OBJ_DIR) $(EXECUTABLE) $(LIB_DIR)

.PHONY: all clean run valgrind
