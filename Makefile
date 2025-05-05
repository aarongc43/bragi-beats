# Makefile

UNITY_DIR := tests/vendor/unity

CC      := clang
CFLAGS  := -std=c11 -Wall -Wextra -O3 -march=native -Iinclude -Isrc -Isrc/core -I$(UNITY_DIR)/src
LDFLAGS :=
SRC     := $(wildcard src/**/*.c)
OBJ     := $(SRC:.c=.o)
TARGET  := bragibeats

# Unity test framework path

UNITY_DIR       := tests/vendor/unity
TEST_SRC        := $(wildcard tests/*.c)
TEST_OBJ        := $(TEST_SRC:.c=.o) $(UNITY_DIR)/src/unity.o
TEST_EXE        := run_tests

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Build unity once
$(UNITY_DIR)/src/unity.o:
	@echo "Building Unity..."
	$(CC) $(CFLAGS) -c $(UNITY_DIR)/src/unity.c -o $@

# test: event system only 
run_tests_event: $(UNITY_DIR)/src/unity.o tests/test_event_system.o src/core/event_system.c
	$(CC) $(CFLAGS) $^ -o $@
	./$@

# test: audio engine only

run_tests_audio: $(UNITY_DIR)/src/unity.o tests/test_audio_engine.o src/audio/audio_engine.o
	$(CC) $(CFLAGS) $^ -o $@
	./$@

run_tests_fft: $(UNITY_DIR)/src/unity.o tests/test_fft.o src/audio/fft/fft.o
	$(CC) $(CFLAGS) $^ -o $@
	./$@

# test runner
test: run_tests_audio run_tests_event run_tests_fft

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) $(TEST_OBJ) $(TEST_EXE)
