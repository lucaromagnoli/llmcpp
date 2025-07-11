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
	@echo "  test           - Run all tests (including integration)"
	@echo "  test-unit      - Run unit tests only (no integration)"
	@echo "  test-parsing   - Run parsing tests only"
	@echo "  test-openai    - Run OpenAI-related tests (no integration)"
	@echo "  test-integration - Run integration tests only"
	@echo "  test-ci        - Run CI-safe tests (excludes integration)"
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
	@echo "  make test-ci             - Run tests suitable for CI"

# Build targets
.PHONY: build
build: configure
	@echo "Building llmcpp..."
	@cmake --build $(BUILD_DIR) --config $(BUILD_TYPE)

.PHONY: debug
debug:
	@$(MAKE) build BUILD_TYPE=Debug

.PHONY: tests
tests: configure-tests
	@echo "Building tests..."
	@cmake --build $(BUILD_DIR)
	@if [ -f .env ]; then cp .env $(BUILD_DIR)/; fi

.PHONY: configure-tests
configure-tests: $(BUILD_DIR)/.created
	@echo "Configuring build system with tests..."
	@cmake -B $(BUILD_DIR) $(CMAKE_ARGS) -DLLMCPP_BUILD_TESTS=ON

# Configuration targets
.PHONY: configure
configure: $(BUILD_DIR)/.created
	@echo "Configuring build system..."
	@cmake -B $(BUILD_DIR) $(CMAKE_ARGS)

$(BUILD_DIR)/.created:
	@mkdir -p $(BUILD_DIR)
	@touch $(BUILD_DIR)/.created

# Testing targets
.PHONY: test
test: tests
	@echo "Running all tests..."
	@cd $(BUILD_DIR) && ./tests/llmcpp_tests

.PHONY: test-unit
test-unit: tests
	@echo "Running unit tests (excluding integration tests)..."
	@cd $(BUILD_DIR) && ./tests/llmcpp_tests "~[integration]"

.PHONY: test-parsing
test-parsing: tests
	@echo "Running parsing tests..."
	@cd $(BUILD_DIR) && ./tests/llmcpp_tests "[parsing]"

.PHONY: test-openai
test-openai: tests
	@echo "Running OpenAI tests (excluding integration)..."
	@cd $(BUILD_DIR) && ./tests/llmcpp_tests "[openai] and ~[integration]"

.PHONY: test-integration
test-integration: tests
	@echo "Running integration tests..."
	@cd $(BUILD_DIR) && ./tests/llmcpp_tests "[integration]"

.PHONY: test-ci
test-ci: tests
	@echo "Running CI-safe tests (excluding integration tests)..."
	@cd $(BUILD_DIR) && ./tests/llmcpp_tests "~[integration]"

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
.PRECIOUS: $(BUILD_DIR)/.created 