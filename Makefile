# Run in MSYS2 UCRT64 on Windows, or Bash on Linux.
# pipefail keeps a failing executable from being hidden by tee.
SHELL := bash
.SHELLFLAGS := -o pipefail -c

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
TEST_DIR = tests
RESULTS_DIR = results

CC = gcc
CPPFLAGS = -I$(INC_DIR) -I$(SRC_DIR)
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -O2
EXEEXT =
ifeq ($(OS),Windows_NT)
EXEEXT = .exe
endif

LIB_SOURCES = $(SRC_DIR)/hc128_init.c $(SRC_DIR)/hc128_core.c \
              $(SRC_DIR)/hc128_stream.c $(SRC_DIR)/hc128_helpers.c
HEADERS = $(INC_DIR)/hc128.h $(SRC_DIR)/hc128_internal.h
TARGET = $(BUILD_DIR)/hc128$(EXEEXT)
KAT_TARGET = $(BUILD_DIR)/test_kat$(EXEEXT)
# Invalid fixtures are not KAT inputs.
KAT_FIXTURES = $(TEST_DIR)/vectors/hc128-kat-001-zero-key-zero-iv.txt \
               $(TEST_DIR)/vectors/hc128-kat-002-zero-key-iv-first-byte-01.txt \
               $(TEST_DIR)/vectors/hc128-kat-003-key-first-byte-55-zero-iv.txt

all: $(TARGET) $(KAT_TARGET)

$(BUILD_DIR) $(RESULTS_DIR):
	mkdir -p "$@"

$(TARGET): app/main.c $(LIB_SOURCES) $(HEADERS) Makefile | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) app/main.c $(LIB_SOURCES) $(LDFLAGS) $(LDLIBS) -o "$@"

$(KAT_TARGET): $(TEST_DIR)/test_kat.c $(LIB_SOURCES) $(HEADERS) Makefile | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_DIR)/test_kat.c $(LIB_SOURCES) $(LDFLAGS) $(LDLIBS) -o "$@"

demo: $(TARGET) | $(RESULTS_DIR)
	./$(TARGET) 2>&1 | tee "$(RESULTS_DIR)/demo_log.txt"

run: demo

test: test-demo test-kat

# Reuse the existing demo/API/stream/core suite and the root demo binary.
test-demo: $(TARGET) | $(RESULTS_DIR)
	+$(MAKE) -f app/Makefile.demo BUILD_DIR="$(BUILD_DIR)" DEMO="$(TARGET)" \
		CC="$(CC)" CPPFLAGS="$(CPPFLAGS)" CFLAGS="$(CFLAGS)" \
		LDFLAGS="$(LDFLAGS)" LDLIBS="$(LDLIBS)" test 2>&1 | tee "$(RESULTS_DIR)/test_log.txt"

test-kat: $(KAT_TARGET) | $(RESULTS_DIR)
	./$(KAT_TARGET) $(KAT_FIXTURES) 2>&1 | tee "$(RESULTS_DIR)/kat_log.txt"

bench: | $(RESULTS_DIR)
	@echo "Running benchmark..."
	@echo "Benchmark placeholder" > $(RESULTS_DIR)/bench_log.txt

# Preserve recorded run logs.
clean:
	rm -rf "$(BUILD_DIR)"

.PHONY: all demo run test test-demo test-kat bench clean
