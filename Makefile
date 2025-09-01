CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
INCLUDES = -Iinclude

# Source files (explicitly listed)
SOURCES = src/core/database.c src/core/btree.c src/operations/crud.c \
          src/storage/storage.c src/utils/utils.c src/interface/repl.c src/main.c

# Object files (in obj directory)
OBJECTS = $(SOURCES:%.c=obj/%.o)

# Target executable
TARGET = coredb

# Test targets
TESTDIR = test
TEST_TARGET = $(TESTDIR)/test_coredb

# Default target
all: $(TARGET)

# Create object directories
obj/src/core/:
	mkdir -p $@

obj/src/operations/:
	mkdir -p $@

obj/src/storage/:
	mkdir -p $@

obj/src/utils/:
	mkdir -p $@

obj/src/interface/:
	mkdir -p $@

obj/src/:
	mkdir -p $@

obj/:
	mkdir -p $@

# Compile source files to object files in obj directory
obj/%.o: %.c | obj/
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

# Test targets - build test executable
test-build: $(TEST_TARGET)

$(TEST_TARGET):
	$(MAKE) -C $(TESTDIR) all

# Test targets - run tests
test: test-build
	$(MAKE) -C $(TESTDIR) test

# Clean build artifacts
clean:
	rm -rf obj $(TARGET)
	$(MAKE) -C $(TESTDIR) clean

# Clean database data
clean-data:
	rm -f coredb.db

# Clean all (including tests)
clean-all: clean
	$(MAKE) -C $(TESTDIR) clean-all

# Install (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Uninstall (optional)
uninstall:
	rm -f /usr/local/bin/$(TARGET)

# Phony targets
.PHONY: all clean clean-data clean-all install uninstall test test-build
