CXX = gcc
CXXFLAGS = -Wall -I./include -g
VALGRINDFLAGS = --leak-check=full --track-origins=yes

SRC_DIR = src
OBJ_DIR = obj
LIB_DIR = lib

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

# Имя библиотеки
LIB_NAME = libcalc.so 
# Короткое имя библиотеки для -l
LIB_SHORT_NAME = $(patsubst lib%,%,$(basename $(LIB_NAME)))
# Берём все исходные файлы кроме main.c
LIB_OBJECTS = $(filter-out $(OBJ_DIR)/main.o, $(OBJECTS)) 

EXECUTABLE = runner

all: $(LIB_DIR)/$(LIB_NAME) $(EXECUTABLE)

$(EXECUTABLE): $(OBJ_DIR)/main.o $(LIB_DIR)/$(LIB_NAME)
	$(CXX) $(OBJ_DIR)/main.o -L$(LIB_DIR) -l$(LIB_SHORT_NAME) -o $@

# Создаём библиотеку в папке lib
# Через -shared создам динамическую библиотеку
$(LIB_DIR)/$(LIB_NAME): $(LIB_OBJECTS)
	@mkdir -p $(LIB_DIR)
	$(CXX) -shared $^ -o $@ 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Используем в make CURDIR вместо PWD, и устанавливаем путь библиотеки
run: $(EXECUTABLE)
	LD_LIBRARY_PATH=$(CURDIR)/lib:$$(LD_LIBRARY_PATH) ./$(EXECUTABLE)
	@echo "Программа '$(EXECUTABLE)' завершена."

# Сделал отдельный запуск с библиотекой через плагин, не стал создавать лишние переменные
run_plugin: $(LIB_DIR)/$(LIB_NAME)
	$(CXX) $(CXXFLAGS) src_plugin/main.c -o runner_plugin -ldl
	LD_LIBRARY_PATH=$(CURDIR)/lib:$$(LD_LIBRARY_PATH) ./runner_plugin

# Нет выделения памяти, проверять нет смысла
# valgrind: $(EXECUTABLE)
# 	valgrind $(VALGRINDFLAGS) ./$(EXECUTABLE)

clean:
	rm -rf $(OBJ_DIR) $(EXECUTABLE) $(LIB_DIR) runner_plugin

.PHONY: all clean run valgrind
