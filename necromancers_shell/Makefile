# Necromancer's Shell - Makefile
# C11 standard, ncurses-based terminal game

# Compiler and flags
CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Werror -pedantic
INCLUDES := -Isrc
LIBS := -lncurses -lm

# Directories
SRC_DIR := src
BUILD_DIR := build
TEST_DIR := tests

# Source files (auto-detected)
CORE_SRC := $(wildcard $(SRC_DIR)/core/*.c)
TERM_SRC := $(wildcard $(SRC_DIR)/terminal/*.c)
UTIL_SRC := $(wildcard $(SRC_DIR)/utils/*.c)
CMD_SRC := $(wildcard $(SRC_DIR)/commands/*.c) $(wildcard $(SRC_DIR)/commands/commands/*.c)
GAME_SRC := $(wildcard $(SRC_DIR)/game/souls/*.c) $(wildcard $(SRC_DIR)/game/resources/*.c) $(wildcard $(SRC_DIR)/game/world/*.c) $(wildcard $(SRC_DIR)/game/minions/*.c) $(SRC_DIR)/game/game_state.c
MAIN_SRC := $(SRC_DIR)/main.c

ALL_SRC := $(CORE_SRC) $(TERM_SRC) $(UTIL_SRC) $(CMD_SRC) $(GAME_SRC) $(MAIN_SRC)
ALL_OBJ := $(ALL_SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Test files
TEST_SRC := $(wildcard $(TEST_DIR)/*.c)
TEST_BIN := $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/test_%,$(TEST_SRC))

# Targets
TARGET := $(BUILD_DIR)/necromancer_shell
TARGET_DEBUG := $(BUILD_DIR)/necromancer_shell_debug

# Default target
.DEFAULT_GOAL := release

# Build modes
.PHONY: all debug release clean test valgrind help

all: debug release

debug: CFLAGS += -g -O0 -DDEBUG -fsanitize=address -fsanitize=undefined
debug: LDFLAGS += -fsanitize=address -fsanitize=undefined
debug: $(TARGET_DEBUG)

release: CFLAGS += -O2 -DNDEBUG -march=native
release: $(TARGET)

# Main targets
$(TARGET): $(ALL_OBJ)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $^ -o $@ $(LIBS)
	@echo "Built release: $(TARGET)"

$(TARGET_DEBUG): $(ALL_OBJ)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $^ -o $@ $(LIBS)
	@echo "Built debug: $(TARGET_DEBUG)"

# Object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Dependency tracking
-include $(ALL_OBJ:.o=.d)

$(BUILD_DIR)/%.d: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(INCLUDES) -MM -MT $(BUILD_DIR)/$*.o $< -MF $@

# Tests
test: $(TEST_BIN)
	@echo "Running tests..."
	@for test in $(TEST_BIN); do \
		echo "Running $$test..."; \
		$$test || exit 1; \
	done
	@echo "All tests passed!"

$(BUILD_DIR)/test_%: $(TEST_DIR)/%.c $(filter-out $(BUILD_DIR)/main.o,$(ALL_OBJ))
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LIBS)

# Memory checking
valgrind: debug
	valgrind --leak-check=full \
	         --show-leak-kinds=all \
	         --track-origins=yes \
	         --verbose \
	         --log-file=valgrind-out.txt \
	         $(TARGET_DEBUG)
	@echo "Valgrind output written to valgrind-out.txt"

# Profiling
profile: CFLAGS += -pg
profile: $(TARGET)
	./$(TARGET)
	gprof $(TARGET) gmon.out > profile.txt
	@echo "Profile written to profile.txt"

# Static analysis
analyze:
	cppcheck --enable=all --inconclusive --std=c11 $(SRC_DIR)

# Formatting
format:
	find $(SRC_DIR) $(TEST_DIR) -name "*.c" -o -name "*.h" | xargs clang-format -i

# Clean
clean:
	rm -rf $(BUILD_DIR)
	rm -f valgrind-out.txt gmon.out profile.txt

# Help
help:
	@echo "Necromancer's Shell - Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make              - Build release version"
	@echo "  make debug        - Build debug version with sanitizers"
	@echo "  make release      - Build optimized release version"
	@echo "  make test         - Build and run all tests"
	@echo "  make valgrind     - Run with valgrind memory checker"
	@echo "  make profile      - Build with profiling, run, and generate profile"
	@echo "  make analyze      - Run static analysis (cppcheck)"
	@echo "  make format       - Format code with clang-format"
	@echo "  make clean        - Remove all build artifacts"
	@echo "  make help         - Show this help"

# Include dependency files
DEPS := $(ALL_OBJ:.o=.d)
-include $(DEPS)
