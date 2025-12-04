CC = gcc
CFLAGS = -Wall -Wextra -std=c99
UNITY_SRC = unity_tests/unity.c

HW3_INC = src
HW3_SOURCE = src/homework_3.c
HW3_MAIN = main/main.c
HW3_TESTS = tests/test_runner.c

all: run

app: $(HW3_SOURCE) $(HW3_MAIN)
	$(CC) $(CFLAGS) -I$(HW3_INC) $^ -o hw3_runner

run: app
	./hw3_runner

tests: $(HW3_SOURCE) $(HW3_TESTS) $(UNITY_SRC)
	$(CC) $(CFLAGS) $^ -o test_runner

run_tests: tests
	./test_runner

clean:
	rm -f hw3_runner test_runner *.o

.PHONY: all app tests run_tests clean
