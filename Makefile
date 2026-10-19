# Khai báo biến đường dẫn
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
TEST_DIR = tests
RESULTS_DIR = results

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I$(INC_DIR)

TARGET = $(BUILD_DIR)/hc128
TEST_TARGET = $(BUILD_DIR)/test_hc128

all: $(BUILD_DIR) $(TARGET) $(TEST_TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(RESULTS_DIR)

$(TARGET):
	@echo "Building main executable..."
	touch $(TARGET)

$(TEST_TARGET): $(TEST_DIR)/test_runner.c
	$(CC) $(CFLAGS) $(TEST_DIR)/test_runner.c -o $(TEST_TARGET)

test: $(TEST_TARGET)
	@echo "Running tests..."
	./$(TEST_TARGET) | tee $(RESULTS_DIR)/test_log.txt

test-kat:
	@echo "Running KAT test vectors..."
	@echo "KAT test placeholder" > $(RESULTS_DIR)/kat_log.txt

bench:
	@echo "Running benchmark..."
	@echo "Benchmark placeholder" > $(RESULTS_DIR)/bench_log.txt

clean:
	rm -rf $(BUILD_DIR) $(RESULTS_DIR)

.PHONY: all test test-kat bench clean