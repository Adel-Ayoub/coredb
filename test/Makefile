CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
INCLUDES = -I../include
SRCDIR = ../src
OBJDIR = ../obj
TESTOBJDIR = obj
TESTDIR = .

# Source files for tests
TEST_SOURCES = test_common.c test_basic_operations.c test_select_by_id.c \
               test_unique_id.c test_input_validation.c test_update.c \
               test_compaction.c test_runner.c

# Object files - tests in test/obj/, project files from main obj directory
TEST_OBJECTS = $(TEST_SOURCES:%.c=$(TESTOBJDIR)/%.o)
PROJECT_OBJECTS = $(OBJDIR)/src/core/database.o $(OBJDIR)/src/core/btree.o \
                  $(OBJDIR)/src/operations/crud.o $(OBJDIR)/src/storage/storage.o \
                  $(OBJDIR)/src/utils/utils.o $(OBJDIR)/src/interface/repl.o

# Test executable
TEST_TARGET = test_coredb

# Default target
all: $(TEST_TARGET)

# Create test object directory
$(TESTOBJDIR)/:
	mkdir -p $@

# Compile test source files (in test/obj directory)
$(TESTOBJDIR)/%.o: %.c | $(TESTOBJDIR)/
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Link test executable
$(TEST_TARGET): $(TEST_OBJECTS) $(PROJECT_OBJECTS)
	$(CC) $(TEST_OBJECTS) $(PROJECT_OBJECTS) -o $(TEST_TARGET)

# Check if main project is built, build it if needed
check-main-project:
	@if [ ! -f ../obj/src/core/database.o ]; then \
		echo "Main project not built. Building it first..."; \
		$(MAKE) -C .. all; \
	fi

# Make test objects depend on main project being built
$(TEST_OBJECTS): check-main-project

# Run tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Clean test artifacts
clean:
	rm -rf $(TESTOBJDIR) $(TEST_TARGET)

# Clean all (including project objects)
clean-all: clean
	rm -f $(OBJDIR)/src/**/*.o

# Phony targets
.PHONY: all test clean clean-all
