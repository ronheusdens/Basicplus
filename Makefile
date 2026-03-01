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

# SDL2 detection and flags
SDL2_CFLAGS := $(shell pkg-config --cflags sdl2 2>/dev/null)
SDL2_LDFLAGS := $(shell pkg-config --libs sdl2 SDL2_ttf 2>/dev/null)
ifneq ($(SDL2_CFLAGS),)
    CFLAGS_COMMON += $(SDL2_CFLAGS) -DUSE_SDL
    LDFLAGS_COMMON += $(SDL2_LDFLAGS)
    SDL2_ENABLED := 1
else
    SDL2_ENABLED := 0
endif

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
	$(SRC_DIR)/common.c \
	$(SRC_DIR)/compat.c

ifeq ($(SDL2_ENABLED),1)
SRCS += $(SRC_DIR)/termio_sdl.c
else
SRCS += $(SRC_DIR)/termio.c
endif

OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Compilation flags for this project
CFLAGS := $(CFLAGS_COMMON)
LDFLAGS := $(LDFLAGS_COMMON)

# Targets
.PHONY: all build test clean help app install-app

all: build

help:
	@echo "Basic++ Interpreter Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make build       - Build interpreter (default)"
	@echo "  make test        - Run entire test suite"
	@echo "  make app         - Create Basic++.app bundle (macOS only)"
	@echo "  make install-app - Install Basic++.app to /Applications (macOS only)"
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

app: build
	@if [ "$(UNAME_S)" != "Darwin" ]; then \
		echo "Error: App bundle can only be built on macOS"; \
		exit 1; \
	fi
	@echo "Creating Basic++ app bundle..."
	@rm -rf "Basic++.app"
	@mkdir -p "Basic++.app/Contents/MacOS"
	@clang -O2 -framework Cocoa -o "Basic++.app/Contents/MacOS/Basic++" macos_app/Launcher.m
	@cp $(BINARY) "Basic++.app/Contents/MacOS/basicpp"
	@chmod +x "Basic++.app/Contents/MacOS/Basic++"
	@chmod +x "Basic++.app/Contents/MacOS/basicpp"
	@printf '<?xml version="1.0" encoding="UTF-8"?>\n<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">\n<plist version="1.0">\n<dict>\n  <key>CFBundleDevelopmentRegion</key>\n  <string>en</string>\n  <key>CFBundleExecutable</key>\n  <string>Basic++</string>\n  <key>CFBundleIdentifier</key>\n  <string>com.ronheusdens.basicpp</string>\n  <key>CFBundleInfoDictionaryVersion</key>\n  <string>6.0</string>\n  <key>CFBundleName</key>\n  <string>Basic++</string>\n  <key>CFBundlePackageType</key>\n  <string>APPL</string>\n  <key>CFBundleShortVersionString</key>\n  <string>1.0</string>\n  <key>CFBundleVersion</key>\n  <string>1</string>\n  <key>NSHighResolutionCapable</key>\n  <true/>\n  <key>LSUIElement</key>\n  <true/>\n  <key>NSHumanReadableCopyright</key>\n  <string>Copyright © 2026. All rights reserved.</string>\n  <key>CFBundleDocumentTypes</key>\n  <array>\n    <dict>\n      <key>CFBundleTypeRole</key>\n      <string>Editor</string>\n      <key>CFBundleTypeExtensions</key>\n      <array>\n        <string>basicpp</string>\n        <string>bas</string>\n      </array>\n      <key>CFBundleTypeName</key>\n      <string>Basic Source File</string>\n      <key>CFBundleTypeOSTypes</key>\n      <array>\n        <string>****</string>\n      </array>\n    </dict>\n  </array>\n</dict>\n</plist>' > "Basic++.app/Contents/Info.plist"
	@echo "App bundle created: Basic++.app"

install-app: app
	@if [ "$(UNAME_S)" != "Darwin" ]; then \
		echo "Error: App can only be installed on macOS"; \
		exit 1; \
	fi
	@echo "Installing Basic++ app to /Applications..."
	@rm -rf "/Applications/Basic++.app"
	@cp -R "Basic++.app" "/Applications/"
	@echo "✓ App installed to /Applications/Basic++.app"
	@echo ""
	@echo "Installing basicpp to /usr/local/bin for file association..."
	@mkdir -p /usr/local/bin
	@cp $(BINARY) /usr/local/bin/basicpp
	@chmod +x /usr/local/bin/basicpp
	@echo "✓ Installed: /usr/local/bin/basicpp"
	@echo ""
	@echo "Registering file associations..."
	@/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister -f "/Applications/Basic++.app" 2>/dev/null
	@echo "✓ File associations registered"
	@echo ""
	@echo "Setup complete! To finish:"
	@echo "  1. Open Finder"
	@echo "  2. Right-click any .basicpp file"
	@echo "  3. Select 'Get Info'"
	@echo "  4. Change 'Open with:' to 'Basic++'"
	@echo "  5. Click 'Change All'"

clean:
	@rm -rf $(OBJ_DIR) $(BUILD_DIR)
	@if [ "$(UNAME_S)" = "Darwin" ]; then \
		rm -rf "Basic++.app"; \
	fi
	@echo "✓ Cleaned build artifacts"

.SILENT: help