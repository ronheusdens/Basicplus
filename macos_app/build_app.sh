#!/bin/bash
# Build macOS app bundle for TRS-80 Level II BASIC

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
APP_NAME="TRS-80 Level II BASIC.app"
APP_DIR="$PROJECT_DIR/$APP_NAME"
BUILD_DIR="$PROJECT_DIR/build"
BIN_NAME="basic-trs80-ast"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Building macOS app bundle...${NC}"

# Check if we're on macOS
if [ "$(uname)" != "Darwin" ]; then
    echo -e "${RED}Error: This script only works on macOS${NC}"
    exit 1
fi

# Build the binary first
echo -e "${YELLOW}Building AST-based BASIC interpreter...${NC}"
cd "$PROJECT_DIR"
make ast-build

if [ ! -f "$BUILD_DIR/bin/$BIN_NAME" ]; then
    echo -e "${RED}Error: Binary not found at $BUILD_DIR/bin/$BIN_NAME${NC}"
    exit 1
fi

# Create app bundle structure
echo -e "${YELLOW}Creating app bundle structure...${NC}"
rm -rf "$APP_DIR"
mkdir -p "$APP_DIR/Contents/MacOS"
mkdir -p "$APP_DIR/Contents/Resources"

# Copy binary
echo -e "${YELLOW}Copying binary...${NC}"
cp "$BUILD_DIR/bin/$BIN_NAME" "$APP_DIR/Contents/MacOS/$BIN_NAME"
chmod +x "$APP_DIR/Contents/MacOS/$BIN_NAME"

# Copy launcher script
echo -e "${YELLOW}Installing launcher...${NC}"
cp "$SCRIPT_DIR/launcher.sh" "$APP_DIR/Contents/MacOS/launcher.sh"
chmod +x "$APP_DIR/Contents/MacOS/launcher.sh"

# Copy Info.plist
echo -e "${YELLOW}Installing Info.plist...${NC}"
cp "$SCRIPT_DIR/Info.plist" "$APP_DIR/Contents/Info.plist"

# Copy icon
echo -e "${YELLOW}Installing icon...${NC}"
if [ -f "$SCRIPT_DIR/AppIcon.png" ]; then
    cp "$SCRIPT_DIR/AppIcon.png" "$APP_DIR/Contents/Resources/AppIcon.png"
    # Update Info.plist to reference the icon
    sed -i.bak 's|<string></string>|<string>AppIcon</string>|' "$APP_DIR/Contents/Info.plist" 2>/dev/null || sed -i '' 's|<string></string>|<string>AppIcon</string>|' "$APP_DIR/Contents/Info.plist"
    rm -f "$APP_DIR/Contents/Info.plist.bak"
    echo -e "${GREEN}Icon installed successfully${NC}"
else
    echo -e "${YELLOW}Note: AppIcon.png not found. Using default app icon.${NC}"
fi
# You can replace this with a custom icon later

# Create a simple text-based icon using iconutil (requires .iconset)
# For now, we'll skip the icon and use the default
echo -e "${YELLOW}Note: Using default app icon. Custom icon can be added later.${NC}"

echo -e "${GREEN}App bundle created at: $APP_DIR${NC}"
echo -e "${GREEN}To install to Applications, run: make install-app${NC}"
