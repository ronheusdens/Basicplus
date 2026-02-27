# Makefile for Basic++ Interpreter
# Line-number-free, procedural BASIC with local scoping
# Supports macOS (ARM/Intel) and Linux (x64)

# Detect OS and architecture
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# Default compiler
CC := gcc

# Common flags
WFLAGS ?= -Wall -Wextra -Werror=implicit-function-declaration
CFLAGS_COMMON := -std=c99 $(WFLAGS) -O2 -g
LDFLAGS_COMMON := -lm

# macOS specific settings
ifeq ($(UNAME_S),Darwin)
    CC := clang
    CFLAGS_COMMON += -D__APPLE__
    # Detect ARM vs Intel on macOS
    ifeq ($(UNAME_M),arm64)
        CFLAGS_COMMON += -arch arm64
        LDFLAGS_COMMON += -arch arm64
    else ifeq ($(UNAME_M),x86_64)
        CFLAGS_COMMON += -arch x86_64
        LDFLAGS_COMMON += -arch x86_64
    endif
endif

# Linux specific settings
ifeq ($(UNAME_S),Linux)
    CFLAGS_COMMON += -D__linux__
    ifeq ($(UNAME_M),x86_64)
        CFLAGS_COMMON += -m64
    endif
endif

# Directories
SRC_DIR := src
OBJ_DIR := obj
BUILD_DIR := build
BIN_DIR := $(BUILD_DIR)/bin

# Output binary
BINARY := $(BIN_DIR)/basicpp

# Source files (Phase 1 essentials)
SRCS := \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/lexer.c \
	$(SRC_DIR)/parser.c \
	$(SRC_DIR)/ast.c \
	$(SRC_DIR)/ast_helpers.c \
	$(SRC_DIR)/executor.c \
	$(SRC_DIR)/runtime.c \
	$(SRC_DIR)/eval.c \
	$(SRC_DIR)/builtins.c \
	$(SRC_DIR)/symtable.c \
	$(SRC_DIR)/errors.c \
	$(SRC_DIR)/termio.c \
	$(SRC_DIR)/common.c \
	$(SRC_DIR)/compat.c

OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Compilation flags for this project
CFLAGS := $(CFLAGS_COMMON)
LDFLAGS := $(LDFLAGS_COMMON)

# Targets
.PHONY: all build test clean help

all: build

help:
	@echo "Basic++ Interpreter Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make build       - Build interpreter (default)"
	@echo "  make test        - Run entire test suite"
	@echo "  make clean       - Remove build artifacts"
	@echo "  make help        - Show this message"
	@echo ""
	@echo "Variables:"
	@echo "  WFLAGS           - C compiler warning flags (default: -Wall -Wextra -Werror=implicit)"
	@echo "  TEST             - Run specific test(s) by number (e.g., make test TEST=01)"
	@echo "                     TEST=25 runs all tests matching 25*.bas"
	@echo ""
	@echo "Examples:"
	@echo "  make test              # Run all 66 tests"
	@echo "  make test TEST=01      # Run test 01_trig.bas"
	@echo "  make test TEST=0       # Run all tests starting with 0 (01-09)"
	@echo "  make test TEST=25      # Run test 25_error_handling.bas"

build: $(BINARY)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BINARY): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) $(LDFLAGS) -o $@
	@echo "✓ Built: $@"

test: build
	@bash tests/basic_tests/run_tests.sh $(BINARY) $(TEST)

clean:
	@rm -rf $(OBJ_DIR) $(BUILD_DIR)
	@echo "✓ Cleaned build artifacts"

.SILENT: help