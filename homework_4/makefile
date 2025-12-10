CXX=gcc
CXXFLAGS=-c -Wall
LDFLAGS=
LDLIBS=

SOURCES=$(wildcard src/*.c)
OBJECTS=$(patsubst src/%.c,%.o,$(SOURCES))
EXECUTABLE=app

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CXX) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

%.o: src/%.c
	$(CXX) $(CXXFLAGS) $< -o $@

run: $(EXECUTABLE)
	./$(EXECUTABLE) 
	@echo "Программа '$(EXECUTABLE)' завершена."

.PHONY: all clean run

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)