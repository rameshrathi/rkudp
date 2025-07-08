# Makefile

CC = gcc
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/rkudp

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/rkudp.c
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

CFLAGS = -Wall -O2

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

all: $(TARGET)

run: all
	$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

