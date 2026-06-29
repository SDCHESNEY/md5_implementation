# MD5 Hash Implementation - Makefile
# Project: md5_implementation
# Description: Build configuration for MD5 hash utility
# Version: 1.0.0

# ==============================================================================
# COMPILER AND BUILD CONFIGURATION
# ==============================================================================

CC := gcc
CFLAGS := -std=c99 -Wall -Wextra -pedantic -O2
DEBUG_FLAGS := -g -O0 -DDEBUG
SANITIZE_FLAGS := -fsanitize=address -fsanitize=undefined
LDFLAGS := -lncurses -lm
LDFLAGS_DEBUG := $(LDFLAGS) $(SANITIZE_FLAGS)

# ==============================================================================
# DIRECTORY STRUCTURE
# ==============================================================================

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
TEST_DIR := tests
DOCS_DIR := docs
FIXTURE_DIR := $(TEST_DIR)/fixtures

# ==============================================================================
# SOURCE FILES
# ==============================================================================

SOURCES := $(SRC_DIR)/main.c \
           $(SRC_DIR)/md5.c \
           $(SRC_DIR)/file_handler.c \
           $(SRC_DIR)/tui.c \
           $(SRC_DIR)/utils.c

HEADERS := $(SRC_DIR)/md5.h \
           $(SRC_DIR)/file_handler.h \
           $(SRC_DIR)/tui.h \
           $(SRC_DIR)/utils.h \
           $(SRC_DIR)/config.h

OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# ==============================================================================
# TEST FILES
# ==============================================================================

TEST_SOURCES := $(TEST_DIR)/test_md5.c \
                $(TEST_DIR)/test_file_handler.c \
                $(TEST_DIR)/test_tui.c \
				$(TEST_DIR)/test_vectors.c \
				$(TEST_DIR)/test_cli.c

TEST_OBJECTS := $(TEST_SOURCES:$(TEST_DIR)/%.c=$(OBJ_DIR)/test_%.o)

# Exclude main.c from test builds
TEST_MAIN_OBJECTS := $(filter-out $(OBJ_DIR)/main.o,$(OBJECTS))

# ==============================================================================
# BUILD TARGETS
# ==============================================================================

TARGET := $(BIN_DIR)/md5
DEBUG_TARGET := $(BIN_DIR)/md5_debug

# ==============================================================================
# INSTALLATION
# ==============================================================================

PREFIX := /usr/local
INSTALL_DIR := $(PREFIX)/bin

# ==============================================================================
# DEFAULT TARGET
# ==============================================================================

.PHONY: all
all: $(TARGET)

# ==============================================================================
# MAIN BUILD TARGETS
# ==============================================================================

# Release build
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	@echo "Linking release binary: $@"
	@$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete: $@"

# Debug build with symbols and sanitizers
.PHONY: debug
debug: CFLAGS := $(DEBUG_FLAGS) -std=c99 -Wall -Wextra -pedantic
debug: LDFLAGS := $(LDFLAGS_DEBUG)
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(OBJECTS) | $(BIN_DIR)
	@echo "Linking debug binary: $@"
	@$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Debug build complete: $@"

# ==============================================================================
# OBJECT FILE COMPILATION
# ==============================================================================

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(OBJ_DIR)
	@echo "Compiling: $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/test_%.o: $(TEST_DIR)/test_%.c $(HEADERS) | $(OBJ_DIR)
	@echo "Compiling test: $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# ==============================================================================
# DIRECTORY CREATION
# ==============================================================================

$(OBJ_DIR) $(BIN_DIR) $(FIXTURE_DIR):
	@mkdir -p $@

# ==============================================================================
# TEST TARGETS
# ==============================================================================

.PHONY: test
test: test-md5 test-file-handler test-tui test-vectors test-cli
	@echo "All test suites passed"

.PHONY: test-md5
test-md5: $(BIN_DIR)/test_md5
	@echo "Running MD5 algorithm tests..."
	@$(BIN_DIR)/test_md5

$(BIN_DIR)/test_md5: $(OBJ_DIR)/md5.o $(OBJ_DIR)/test_md5.o $(OBJ_DIR)/utils.o | $(BIN_DIR)
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY: test-file-handler
test-file-handler: $(BIN_DIR)/test_file_handler
	@echo "Running file handler tests..."
	@$(BIN_DIR)/test_file_handler

$(BIN_DIR)/test_file_handler: $(OBJ_DIR)/file_handler.o $(OBJ_DIR)/md5.o $(OBJ_DIR)/test_file_handler.o $(OBJ_DIR)/utils.o | $(BIN_DIR)
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY: test-tui
test-tui: $(BIN_DIR)/test_tui
	@echo "Running TUI tests..."
	@$(BIN_DIR)/test_tui

$(BIN_DIR)/test_tui: $(OBJ_DIR)/tui.o $(OBJ_DIR)/test_tui.o $(OBJ_DIR)/utils.o | $(BIN_DIR)
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY: test-vectors
test-vectors: $(BIN_DIR)/test_vectors
	@echo "Running RFC 1321 test vectors..."
	@$(BIN_DIR)/test_vectors

$(BIN_DIR)/test_vectors: $(OBJ_DIR)/md5.o $(OBJ_DIR)/test_vectors.o $(OBJ_DIR)/utils.o | $(BIN_DIR)
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY: test-cli
test-cli: $(TARGET) $(BIN_DIR)/test_cli
	@echo "Running CLI integration tests..."
	@$(BIN_DIR)/test_cli

$(BIN_DIR)/test_cli: $(OBJ_DIR)/test_cli.o | $(BIN_DIR)
	@$(CC) $(CFLAGS) $^ -o $@

# ==============================================================================
# CODE COVERAGE
# ==============================================================================

.PHONY: coverage
coverage: CFLAGS := $(DEBUG_FLAGS) --coverage -std=c99 -Wall -Wextra -pedantic
coverage: LDFLAGS := -lncurses -lm --coverage
coverage: clean $(TARGET)
	@echo "Generating coverage report..."
	@gcov $(SOURCES) 2>/dev/null || true
	@lcov --directory $(OBJ_DIR) --capture --output-file coverage.info
	@genhtml coverage.info --output-directory coverage_report
	@echo "Coverage report generated in coverage_report/index.html"

# ==============================================================================
# MEMORY LEAK DETECTION
# ==============================================================================

.PHONY: test-valgrind
test-valgrind: $(TARGET)
	@command -v valgrind >/dev/null 2>&1 || { echo "valgrind not found"; exit 1; }
	@echo "Running valgrind memory check..."
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
	          --verbose $(TARGET) -q -t "test"

# ==============================================================================
# PERFORMANCE BENCHMARKS
# ==============================================================================

.PHONY: benchmark-setup
benchmark-setup:
	@echo "Setting up benchmark files..."
	@dd if=/dev/zero of=$(BIN_DIR)/benchmark_1mb.bin bs=1M count=1 2>/dev/null
	@dd if=/dev/zero of=$(BIN_DIR)/benchmark_100mb.bin bs=1M count=100 2>/dev/null
	@echo "Benchmark files created in $(BIN_DIR)/"

.PHONY: benchmark-large-file
benchmark-large-file: $(TARGET) benchmark-setup
	@echo "Benchmarking large file hashing (100 MB)..."
	@time $(TARGET) -f $(BIN_DIR)/benchmark_100mb.bin

.PHONY: benchmark-memory
benchmark-memory: $(TARGET)
	@echo "Testing memory usage..."
	@/usr/bin/time -v $(TARGET) -f $(BIN_DIR)/benchmark_1mb.bin 2>&1 | grep -E "Maximum resident|Elapsed"

# ==============================================================================
# TEST FIXTURES
# ==============================================================================

.PHONY: test-fixtures
test-fixtures: | $(FIXTURE_DIR)
	@echo "Creating test fixtures..."
	@touch $(FIXTURE_DIR)/empty.txt
	@echo -n "a" > $(FIXTURE_DIR)/single_byte.bin
	@dd if=/dev/urandom of=$(FIXTURE_DIR)/small_text.txt bs=100 count=1 2>/dev/null
	@dd if=/dev/urandom of=$(FIXTURE_DIR)/medium_text.txt bs=1M count=1 2>/dev/null
	@dd if=/dev/urandom of=$(FIXTURE_DIR)/large_binary.bin bs=1M count=10 2>/dev/null
	@printf "Hello, World! \xc3\xa9\xc3\xa7\xc3\xa0 UTF-8" > $(FIXTURE_DIR)/unicode_text.txt
	@dd if=/dev/urandom of=$(FIXTURE_DIR)/null_bytes.bin bs=100 count=1 2>/dev/null
	@echo "Test fixtures created in $(FIXTURE_DIR)/"

# ==============================================================================
# INSTALLATION TARGETS
# ==============================================================================

.PHONY: install
install: $(TARGET)
	@echo "Installing $(TARGET) to $(INSTALL_DIR)/md5"
	@mkdir -p $(INSTALL_DIR)
	@cp $(TARGET) $(INSTALL_DIR)/md5
	@chmod 755 $(INSTALL_DIR)/md5
	@echo "Installation complete. Binary available as 'md5'"

.PHONY: uninstall
uninstall:
	@echo "Uninstalling md5 from $(INSTALL_DIR)"
	@rm -f $(INSTALL_DIR)/md5
	@echo "Uninstallation complete"

# ==============================================================================
# DOCUMENTATION
# ==============================================================================

.PHONY: docs
docs:
	@command -v doxygen >/dev/null 2>&1 || { echo "doxygen not found"; exit 1; }
	@echo "Generating API documentation..."
	@doxygen Doxyfile
	@echo "Documentation generated in $(DOCS_DIR)/html/index.html"

# ==============================================================================
# CLEANUP TARGETS
# ==============================================================================

.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR) *.gcov *.gcda *.gcno coverage.info coverage_report
	@echo "Clean complete"

.PHONY: distclean
distclean: clean
	@echo "Removing generated files..."
	@rm -rf $(DOCS_DIR)/html $(DOCS_DIR)/latex
	@rm -f $(FIXTURE_DIR)/*
	@echo "Distribution clean complete"

.PHONY: clean-benchmarks
clean-benchmarks:
	@echo "Removing benchmark files..."
	@rm -f $(BIN_DIR)/benchmark_*.bin
	@echo "Benchmarks cleaned"

# ==============================================================================
# HELP TARGET
# ==============================================================================

.PHONY: help
help:
	@echo "MD5 Hash Implementation - Build Targets"
	@echo "========================================"
	@echo ""
	@echo "Main Targets:"
	@echo "  make                    Build release binary (default)"
	@echo "  make debug              Build debug binary with sanitizers"
	@echo "  make clean              Remove build artifacts"
	@echo "  make distclean          Remove all generated files"
	@echo ""
	@echo "Testing Targets:"
	@echo "  make test               Run all tests"
	@echo "  make test-md5           Run MD5 algorithm tests"
	@echo "  make test-file-handler  Run file handler tests"
	@echo "  make test-tui           Run TUI tests"
	@echo "  make test-vectors       Run RFC 1321 test vectors"
	@echo "  make test-valgrind      Run tests with memory checking"
	@echo ""
	@echo "Coverage & Benchmarking:"
	@echo "  make coverage           Generate code coverage report"
	@echo "  make benchmark-large-file  Benchmark 100MB file hashing"
	@echo "  make benchmark-memory   Test memory usage"
	@echo "  make benchmark-setup    Create benchmark files"
	@echo ""
	@echo "Installation:"
	@echo "  make install            Install binary to system"
	@echo "  make uninstall          Remove installed binary"
	@echo ""
	@echo "Documentation:"
	@echo "  make docs               Generate API documentation"
	@echo "  make test-fixtures      Create test fixture files"
	@echo ""
	@echo "  make help               Display this help message"

# ==============================================================================
# PHONY TARGETS
# ==============================================================================

.PHONY: all debug test coverage install uninstall docs clean distclean help

# ==============================================================================
# SILENT RULE SUFFIX
# ==============================================================================

.SILENT: help
