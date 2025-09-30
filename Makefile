PROJECT_NAME := interaction_combinators
CC := cc
CSTD ?= c11

SRC_DIR := src
INC_DIR := include
BUILD_DIR := build
BIN_DIR := bin

SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
DEPS := $(OBJECTS:.o=.d)

CFLAGS_COMMON := -std=$(CSTD) -I$(INC_DIR) -Wall -Wextra -pedantic -MMD -MP
CFLAGS_DEBUG := $(CFLAGS_COMMON) -O0 -g
CFLAGS_RELEASE := $(CFLAGS_COMMON) -O2

LDFLAGS :=

MODE ?= debug
ifeq ($(MODE),debug)
  CFLAGS := $(CFLAGS_DEBUG)
else ifeq ($(MODE),release)
  CFLAGS := $(CFLAGS_RELEASE)
else
  $(error Unknown MODE '$(MODE)'. Use MODE=debug or MODE=release)
endif

TARGET := $(BIN_DIR)/$(PROJECT_NAME)

.PHONY: all debug release run clean dirs

all: debug

debug: MODE=debug
debug: dirs $(TARGET)

release: MODE=release
release: dirs $(TARGET)

dirs:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | dirs
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPS)


