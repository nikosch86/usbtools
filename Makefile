# USB Tools Makefile
# C implementation of USB reset tool

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = -lusb-1.0

# Targets
TARGET_DIR = build
TARGET = usbtools
OLD_TARGET = usbreset
SRCS = main.c
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Create build directory
$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

# Build main executable
$(TARGET): $(TARGET_DIR) $(OBJS)
	$(CC) $(OBJS) -o $(TARGET_DIR)/$(TARGET) $(LDFLAGS)

# Build old usbreset utility (for compatibility)
$(OLD_TARGET): $(TARGET_DIR) usbreset.c
	$(CC) $(CFLAGS) usbreset.c -o $(TARGET_DIR)/$(OLD_TARGET)

# Compile object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Install (requires sudo)
install: $(TARGET)
	@echo "Installing $(TARGET) to /usr/local/bin"
	@sudo cp $(TARGET_DIR)/$(TARGET) /usr/local/bin/
	@sudo chmod +x /usr/local/bin/$(TARGET)
	@echo "Installation complete"

# Uninstall
uninstall:
	@echo "Removing $(TARGET) from /usr/local/bin"
	@sudo rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstall complete"

# Clean build artifacts
clean:
	rm -f $(OBJS) $(TARGET_DIR)/$(TARGET) $(TARGET_DIR)/$(OLD_TARGET)
	rm -f *.o
	rmdir $(TARGET_DIR) 2>/dev/null || true

# Run the tool (requires sudo)
run: $(TARGET)
	@echo "Running USB Tools (requires root privileges)"
	sudo ./$(TARGET_DIR)/$(TARGET)

# Test build (compile check)
test: $(TARGET)
	@echo "Build test passed - binary created successfully"
	@echo "Testing binary exists..."
	@test -f $(TARGET_DIR)/$(TARGET) && echo "✓ Binary exists"
	@echo "Testing binary is executable..."
	@test -x $(TARGET_DIR)/$(TARGET) && echo "✓ Binary is executable"
	@echo "Checking library dependencies..."
	@ldd $(TARGET_DIR)/$(TARGET) | grep -q libusb && echo "✓ libusb linked correctly"
	@echo "All tests passed"

# Check dependencies
deps:
	@echo "Checking build dependencies..."
	@which gcc > /dev/null && echo "✓ gcc installed" || echo "✗ gcc not found"
	@pkg-config --exists libusb-1.0 && echo "✓ libusb-1.0 installed" || echo "✗ libusb-1.0 not found"
	@echo ""
	@echo "If libusb-1.0 is missing, install it with:"
	@echo "  Ubuntu/Debian: sudo apt-get install libusb-1.0-0-dev"
	@echo "  Fedora/RHEL:   sudo dnf install libusb1-devel"
	@echo "  Arch:          sudo pacman -S libusb"
	@echo "  macOS:         brew install libusb"

# Development build with debug symbols
debug: CFLAGS += -DDEBUG -g3
debug: $(TARGET)
	@echo "Debug build complete"

# Static analysis
lint:
	@echo "Running static analysis..."
	@which cppcheck > /dev/null && cppcheck --enable=all --suppress=missingIncludeSystem main.c || echo "cppcheck not installed, skipping"
	@which clang-tidy > /dev/null && clang-tidy main.c -- $(CFLAGS) || echo "clang-tidy not installed, skipping"

# Format code
format:
	@which clang-format > /dev/null && clang-format -i main.c || echo "clang-format not installed"
	@echo "Code formatting complete"

# Help
help:
	@echo "USB Tools Makefile Commands:"
	@echo "  make          - Build the USB tools binary"
	@echo "  make run      - Build and run (requires sudo)"
	@echo "  make test     - Run build tests"
	@echo "  make deps     - Check build dependencies"
	@echo "  make install  - Install to /usr/local/bin (requires sudo)"
	@echo "  make uninstall- Remove from /usr/local/bin (requires sudo)"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make debug    - Build with debug symbols"
	@echo "  make lint     - Run static analysis"
	@echo "  make format   - Format code with clang-format"
	@echo "  make help     - Show this help message"

.PHONY: all clean install uninstall run test deps debug lint format help