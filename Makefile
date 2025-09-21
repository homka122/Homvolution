SRC = src
INCLUDE = include
BUILD = build
DEPS = deps

CFLAGS = -Wall -Wpedantic -Wextra -I$(INCLUDE) -I$(DEPS) -g -fopenmp
LDFLAGS = -lm -pthread

TEST_FRAMEWORK = -lcmocka

$(BUILD)/homv_matrix.o: $(SRC)/homv_matrix.c $(INCLUDE)/homv_matrix.h $(INCLUDE)/homv_core.h
	gcc $(CFLAGS) -c $< -o $@

$(BUILD)/core.o: $(SRC)/core.c $(INCLUDE)/homv_matrix.h $(INCLUDE)/homv_core.h
	gcc $(CFLAGS) -c $< -o $@

$(BUILD)/cli.o: $(SRC)/cli.c $(INCLUDE)/homv_matrix.h $(INCLUDE)/homv_core.h
	gcc $(CFLAGS) -c $< -o $@

$(BUILD)/queue.o: $(SRC)/queue.c
	gcc $(CFLAGS) -c $< -o $@

$(BUILD)/benchmark.o: $(SRC)/benchmark.c
	gcc $(CFLAGS) -c $< -o $@

build-cli: $(BUILD)/cli.o $(BUILD)/homv_matrix.o $(BUILD)/core.o $(BUILD)/queue.o
	gcc $(CFLAGS) $^ $(LDFLAGS) -o $(BUILD)/app

build-benchmark: $(BUILD)/homv_matrix.o $(BUILD)/core.o $(BUILD)/queue.o $(BUILD)/benchmark.o
	gcc $(CFLAGS) $^ $(LDFLAGS) -o $(BUILD)/bench

bench: build-benchmark
	$(BUILD)/bench outline input/limons.jpg

tests: build-cli
	gcc $(SRC)/core.c $(SRC)/homv_matrix.c $(SRC)/queue.c tests/test_methods.c $(CFLAGS) $(LDFLAGS) -o $(BUILD)/test_methods $(TEST_FRAMEWORK)
	gcc $(SRC)/queue.c tests/test_queue.c $(CFLAGS) -o $(BUILD)/test_queue $(TEST_FRAMEWORK)
	$(BUILD)/test_methods
	$(BUILD)/test_queue

format-check:
	clang-format --dry-run --Werror $(SRC)/*.c $(INCLUDE)/*.h
