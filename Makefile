# Makefile for llmcpp - Modern C++20 LLM API Library
# Simplified version with essential targets only

# Configuration
BUILD_DIR := build
BUILD_TYPE := Release
CMAKE_ARGS := -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -G Ninja

# Default target
.PHONY: all
all: build

# Help target
.PHONY: help
help:
	@echo "llmcpp Makefile - Available targets:"
	@echo ""
	@echo "Building:"
	@echo "  build          - Build the library (default)"
	@echo "  debug          - Build in Debug mode"
	@echo "  tests          - Build with tests"
	@echo ""
	@echo "Testing:"
	@echo "  test           - Run tests"
	@echo ""
	@echo "Code Quality:"
	@echo "  format         - Format code with clang-format"
	@echo ""
	@echo "Maintenance:"
	@echo "  clean          - Clean build directory"
	@echo ""
	@echo "Examples:"
	@echo "  make debug test          - Build in debug mode and run tests"
	@echo "  make BUILD_TYPE=Debug    - Build in debug mode"

# Build targets
.PHONY: build
build: configure
	@echo "Building llmcpp..."
	@cmake --build $(BUILD_DIR) --config $(BUILD_TYPE)

.PHONY: debug
debug:
	@$(MAKE) build BUILD_TYPE=Debug

.PHONY: tests
tests: CMAKE_ARGS += -DLLMCPP_BUILD_TESTS=ON
tests: build

# Configuration targets
.PHONY: configure
configure: $(BUILD_DIR)
	@echo "Configuring build system..."
	@cmake -B $(BUILD_DIR) $(CMAKE_ARGS)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Testing targets
.PHONY: test
test: tests
	@echo "Running tests..."
	@cd $(BUILD_DIR) && ctest --output-on-failure -C $(BUILD_TYPE)

# Code quality targets
.PHONY: format
format:
	@echo "Formatting code..."
	@find include src tests -name "*.h" -o -name "*.hpp" -o -name "*.cpp" -o -name "*.c" | \
		xargs clang-format -i --style=file

# Maintenance targets
.PHONY: clean
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)

# Prevent intermediate files from being deleted
.PRECIOUS: $(BUILD_DIR) 