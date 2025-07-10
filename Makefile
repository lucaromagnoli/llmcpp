# Makefile for llmcpp - Modern C++20 LLM API Library
# This Makefile wraps CMake for convenient development workflows

# Configuration
BUILD_DIR := build
BUILD_TYPE := Release
CMAKE_ARGS := -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
CMAKE_GENERATOR := Ninja

# Detect if ninja is available, fallback to Unix Makefiles
ifneq ($(shell which ninja 2>/dev/null),)
	CMAKE_ARGS += -G "$(CMAKE_GENERATOR)"
endif

# Default target
.PHONY: all
all: build

# Help target
.PHONY: help
help:
	@echo "llmcpp Makefile - Available targets:"
	@echo ""
	@echo "Building:"
	@echo "  build          - Build the library (default, Release mode)"
	@echo "  debug          - Build in Debug mode"
	@echo "  release        - Build in Release mode"
	@echo "  examples       - Build with examples"
	@echo "  tests          - Build with tests"
	@echo "  all-features   - Build with tests and examples"
	@echo ""
	@echo "Testing:"
	@echo "  test           - Run tests"
	@echo "  test-verbose   - Run tests with verbose output"
	@echo "  coverage       - Generate coverage report (requires lcov)"
	@echo ""
	@echo "Code Quality:"
	@echo "  format         - Format code with clang-format"
	@echo "  format-check   - Check code formatting"
	@echo "  lint           - Run clang-tidy static analysis"
	@echo "  cppcheck       - Run cppcheck static analysis"
	@echo ""
	@echo "Maintenance:"
	@echo "  clean          - Clean build directory"
	@echo "  clean-all      - Clean everything including dependencies"
	@echo "  install        - Install the library"
	@echo "  uninstall      - Uninstall the library"
	@echo ""
	@echo "Documentation:"
	@echo "  docs           - Generate documentation (requires doxygen)"
	@echo ""
	@echo "Development:"
	@echo "  deps           - Download/update dependencies"
	@echo "  configure      - Configure build system only"
	@echo "  reconfigure    - Reconfigure from scratch"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD_TYPE     - Build type (Debug|Release|RelWithDebInfo) [$(BUILD_TYPE)]"
	@echo "  BUILD_DIR      - Build directory [$(BUILD_DIR)]"
	@echo ""
	@echo "Examples:"
	@echo "  make debug test          - Build in debug mode and run tests"
	@echo "  make BUILD_TYPE=Debug    - Build in debug mode"
	@echo "  make all-features test   - Build everything and test"

# Build targets
.PHONY: build
build: configure
	@echo "Building llmcpp..."
	@cmake --build $(BUILD_DIR) --config $(BUILD_TYPE)

.PHONY: debug
debug:
	@$(MAKE) build BUILD_TYPE=Debug

.PHONY: release
release:
	@$(MAKE) build BUILD_TYPE=Release

.PHONY: examples
examples: CMAKE_ARGS += -DLLMCPP_BUILD_EXAMPLES=ON
examples: build

.PHONY: tests
tests: CMAKE_ARGS += -DLLMCPP_BUILD_TESTS=ON
tests: build

.PHONY: all-features
all-features: CMAKE_ARGS += -DLLMCPP_BUILD_TESTS=ON -DLLMCPP_BUILD_EXAMPLES=ON
all-features: build

# Configuration targets
.PHONY: configure
configure: $(BUILD_DIR)
	@echo "Configuring build system..."
	@cmake -B $(BUILD_DIR) $(CMAKE_ARGS)

.PHONY: reconfigure
reconfigure: clean configure

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Testing targets
.PHONY: test
test: tests
	@echo "Running tests..."
	@cd $(BUILD_DIR) && ctest --output-on-failure -C $(BUILD_TYPE)

.PHONY: test-verbose
test-verbose: tests
	@echo "Running tests (verbose)..."
	@cd $(BUILD_DIR) && ctest --verbose --output-on-failure -C $(BUILD_TYPE)

.PHONY: coverage
coverage: CMAKE_ARGS += -DCMAKE_CXX_FLAGS="--coverage" -DCMAKE_EXE_LINKER_FLAGS="--coverage"
coverage: BUILD_TYPE := Debug
coverage: tests
	@echo "Generating coverage report..."
	@cd $(BUILD_DIR) && ctest --output-on-failure
	@lcov --directory $(BUILD_DIR) --capture --output-file $(BUILD_DIR)/coverage.info
	@lcov --remove $(BUILD_DIR)/coverage.info '/usr/*' --output-file $(BUILD_DIR)/coverage.info
	@lcov --remove $(BUILD_DIR)/coverage.info '*/build/_deps/*' --output-file $(BUILD_DIR)/coverage.info
	@lcov --list $(BUILD_DIR)/coverage.info
	@genhtml $(BUILD_DIR)/coverage.info --output-directory $(BUILD_DIR)/coverage_html
	@echo "Coverage report generated in $(BUILD_DIR)/coverage_html/index.html"

# Code quality targets
.PHONY: format
format:
	@echo "Formatting code..."
	@find include src tests examples -name "*.h" -o -name "*.hpp" -o -name "*.cpp" -o -name "*.c" | \
		xargs clang-format -i --style=file

.PHONY: format-check
format-check:
	@echo "Checking code formatting..."
	@find include src tests examples -name "*.h" -o -name "*.hpp" -o -name "*.cpp" -o -name "*.c" | \
		xargs clang-format --dry-run --Werror --style=file

.PHONY: lint
lint: configure
	@echo "Running clang-tidy..."
	@cd $(BUILD_DIR) && run-clang-tidy -p . -header-filter=".*" ../include/ ../src/

.PHONY: cppcheck
cppcheck:
	@echo "Running cppcheck..."
	@cppcheck --enable=all --inconclusive --inline-suppr \
		--suppress=missingIncludeSystem \
		--suppress=unmatchedSuppression \
		--suppress=unusedFunction \
		--error-exitcode=1 \
		-I include/ \
		src/ include/

# Installation targets
.PHONY: install
install: build
	@echo "Installing llmcpp..."
	@cmake --install $(BUILD_DIR) --config $(BUILD_TYPE)

.PHONY: uninstall
uninstall:
	@echo "Uninstalling llmcpp..."
	@if [ -f $(BUILD_DIR)/install_manifest.txt ]; then \
		cat $(BUILD_DIR)/install_manifest.txt | xargs rm -f; \
	else \
		echo "No install manifest found. Cannot uninstall."; \
	fi

# Maintenance targets
.PHONY: clean
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)

.PHONY: clean-all
clean-all: clean
	@echo "Cleaning all generated files..."
	@rm -rf docs/html docs/latex

.PHONY: deps
deps: configure
	@echo "Dependencies are automatically managed by CMake FetchContent"

# Documentation target
.PHONY: docs
docs:
	@echo "Generating documentation..."
	@if command -v doxygen >/dev/null 2>&1; then \
		mkdir -p docs; \
		doxygen docs/Doxyfile 2>/dev/null || echo "Doxyfile not found - skipping documentation"; \
	else \
		echo "Doxygen not found. Please install doxygen to generate documentation."; \
	fi

# Utility targets
.PHONY: info
info:
	@echo "llmcpp Build Information:"
	@echo "  Build Type: $(BUILD_TYPE)"
	@echo "  Build Dir:  $(BUILD_DIR)"
	@echo "  CMake Args: $(CMAKE_ARGS)"
	@echo ""
	@echo "System Information:"
	@echo "  OS: $$(uname -s)"
	@echo "  Arch: $$(uname -m)"
	@echo "  CMake: $$(cmake --version | head -n1 || echo 'Not found')"
	@echo "  Ninja: $$(ninja --version 2>/dev/null || echo 'Not found')"
	@echo "  Clang-format: $$(clang-format --version | head -n1 || echo 'Not found')"
	@echo "  Clang-tidy: $$(clang-tidy --version | head -n1 || echo 'Not found')"

# Package targets
.PHONY: package
package: build
	@echo "Creating package..."
	@cd $(BUILD_DIR) && cpack

.PHONY: package-source
package-source:
	@echo "Creating source package..."
	@git archive --format=tar.gz --prefix=llmcpp-$$(git describe --tags --always)/ HEAD > llmcpp-source.tar.gz
	@echo "Source package created: llmcpp-source.tar.gz"

# Development shortcuts
.PHONY: dev
dev: debug examples tests

.PHONY: ci
ci: all-features test format-check lint

# Quick build and test cycle
.PHONY: quick
quick:
	@$(MAKE) debug tests test

# Prevent make from interpreting arguments as files
%:
	@: 