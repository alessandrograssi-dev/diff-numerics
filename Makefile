# Makefile
# -------------------------------------------------------------
# Top-level Makefile for the diff-numerics project.
#
# Provides convenient targets for building, installing, uninstalling,
# and testing the project using CMake and Make.
#
# Main targets:
#   all        - Build the project
#   install    - Install the binary, headers, and man page
#   uninstall  - Remove installed files
#   test       - Run the test suite using CTest
#   clean      - Remove build artifacts
# -------------------------------------------------------------

# Directories for build and binary files
BUILD_DIR := build

# CMake and Make commands
CMAKE := cmake
MAKE := make

# Phony targets that do not correspond to files
.PHONY: all clean install install-manual uninstall uninstall-cmake help

# Default target: build everything
all: $(BUILD_DIR)
	cd $(BUILD_DIR) && $(MAKE) -j

# Configure project and create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && $(CMAKE) .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Remove build and bin directories
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Configure Release and install binary/man page
install:
	cd $(BUILD_DIR) && $(CMAKE) -DCMAKE_BUILD_TYPE=Release ..
	$(CMAKE) --install $(BUILD_DIR)

# Manual install of binary and man page to /usr/local
install-manual:
	install -Dm755 bin/diff-numerics /usr/local/bin/diff-numerics
	install -Dm644 diff-numerics.1 /usr/local/share/man/man1/diff-numerics.1

# Remove installed binary and man page (manual)
uninstall:
	@echo "Uninstalling diff-numerics and man page..."
	rm -f /usr/local/bin/diff-numerics
	rm -f /usr/local/share/man/man1/diff-numerics.1
	mandb

# Remove installed files using CMake script
uninstall-cmake:
	cmake -P cmake_uninstall.cmake

# Build and run all tests (with output)
test: all
	cd $(BUILD_DIR) && $(MAKE) && ctest --output-on-failure

# Show available targets and their descriptions
help:
	@echo "Available targets:"
	@echo "  all              - Build the project (default: Debug)"
	@echo "  install          - Configure Release and install binary/man page (recommended)"
	@echo "  install-manual   - Manual install of binary and man page to /usr/local"
	@echo "  uninstall        - Remove installed binary and man page (manual)"
	@echo "  uninstall-cmake  - Remove installed files using CMake script"
	@echo "  test             - Build and run all tests (with output)"
	@echo "  clean            - Remove build and bin directories"
	@echo "  help             - Show this help message"