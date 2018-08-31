# Project
PROJECT_NAME=bw

# Directories
SOURCE_DIR=src
TEST_DIR=test
BUILD_DIR=build

OBJECT_DIR=$(BUILD_DIR)/obj
DEPEND_DIR=$(BUILD_DIR)/dep
BIN_DIR=$(BUILD_DIR)/bin

# Automatic variables

SOURCE_FILES=$(wildcard $(SOURCE_DIR)/*.c)
OBJECT_FILES=$(patsubst $(SOURCE_DIR)/%.c,$(OBJECT_DIR)/%.o,$(SOURCE_FILES))
DEPEND_FILES=$(patsubst $(SOURCE_DIR)/%.c,$(DEPEND_DIR)/%.d,$(SOURCE_FILES))
MAIN=$(OBJECT_DIR)/$(PROJECT_NAME).o
EXE=$(BIN_DIR)/$(PROJECT_NAME)

TEST_SOURCE_FILES=$(wildcard $(TEST_DIR)/*.c)
TEST_OBJECT_FILES=$(patsubst $(TEST_DIR)/%.c,$(OBJECT_DIR)/%.o,$(TEST_SOURCE_FILES))
TEST_DEPEND_FILES=$(patsubst $(TEST_DIR)/%.c,$(DEPEND_DIR)/%.d,$(TEST_SOURCE_FILES))
TEST_MAIN=$(OBJECT)/$(PROJECT_NAME)-test.o
TEST_EXE=$(BIN_DIR)/$(PROJECT_NAME)-test

# Compiler options

CFLAGS+=-Wall -Werror
LDFLAGS+=-Wall -Werror

$(TEST_OBJECT_FILES) $(TEST_DEPEND_FILES): CFLAGS+=-I$(SOURCE_DIR)
$(TEST_OBJECT_FILES) $(TEST_EXE): LDFLAGS+=-lcheck

# Targets

.PHONY: all
all: $(EXE) $(TEST_EXE) check

.PHONY: check
check: $(TEST_EXE)
	@$(TEST_EXE)

# Executables

$(EXE): $(OBJECT_FILES)
$(TEST_EXE): $(filter-out $(MAIN),$(OBJECT_FILES)) $(TEST_OBJECT_FILES)

$(EXE) $(TEST_EXE): | $(BIN_DIR)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

# Object files

.SECONDEXPANSION:
$(OBJECT_FILES): $$(patsubst $(OBJECT_DIR)/%.o,$(SOURCE_DIR)/%.c,$$@)
.SECONDEXPANSION:
$(TEST_OBJECT_FILES): $$(patsubst $(OBJECT_DIR)/%.o,$(TEST_DIR)/%.c,$$@)

$(OBJECT_FILES) $(TEST_OBJECT_FILES): | $(OBJECT_DIR)
	$(CC) -c $< $(CFLAGS) -o $@

# Dependencies

.SECONDEXPANSION:
$(DEPEND_FILES): $$(patsubst $(DEPEND_DIR)/%.d,$(SOURCE_DIR)/%.c,$$@)
.SECONDEXPANSION:
$(TEST_DEPEND_FILES): $$(patsubst $(DEPEND_DIR)/%.d,$(TEST_DIR)/%.c,$$@)

.PRECIOUS: $(DEPEND_FILES) $(TEST_DEPEND_FILES)
$(DEPEND_FILES) $(TEST_DEPEND_FILES): | $(DEPEND_DIR)
	@echo Generating $@
	@# Generate a dependencies file
	@$(CC) $(CFLAGS) -MM $< -MT $(patsubst $(DEPEND_DIR)/%.d,$(OBJECT_DIR)/%.o,$@) -MG -o $@

include $(DEPEND_FILES) $(TEST_DEPEND_FILES)

# Directories

$(BUILD_DIR) $(OBJECT_DIR) $(DEPEND_DIR) $(BIN_DIR):
	mkdir -p $@

# Clean

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
