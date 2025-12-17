CXX=gcc
CXXFLAGS=-c -Wall

SOURCES=$(wildcard src/*.c)
OBJECTS=$(patsubst src/%.c,%.o,$(SOURCES))
EXECUTABLE=runner

all: run

$(EXECUTABLE): $(OBJECTS) 
	$(CXX) $(OBJECTS) -o $@

%.o: src/%.c
	$(CXX) $(CXXFLAGS) $< -o $@

run: $(EXECUTABLE)
	./$(EXECUTABLE) 
	@echo "Программа '$(EXECUTABLE)' завершена."

.PHONY: all clean run

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)