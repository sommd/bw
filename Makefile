# Project
PROJECT_NAME=bw
PROJECT_VERSION=1.0
PROJECT_BUGREPORT=<https://github.com/sommd/bw/issues/new>

# Directories
SOURCE_DIR=src
TEST_DIR=test
BUILD_DIR=build

OBJECT_DIR=$(BUILD_DIR)/obj
GEN_DIR=$(BUILD_DIR)/gen
DEPEND_DIR=$(BUILD_DIR)/dep
BIN_DIR=$(BUILD_DIR)/bin

# Automatic variables

SOURCE_FILES=$(wildcard $(SOURCE_DIR)/*.c)
OBJECT_FILES=$(patsubst $(SOURCE_DIR)/%.c,$(OBJECT_DIR)/%.o,$(SOURCE_FILES))
GEN_FILES=$(patsubst $(SOURCE_DIR)/%.in,$(GEN_DIR)/%,$(wildcard $(SOURCE_DIR)/*.in))
DEPEND_FILES=$(patsubst $(SOURCE_DIR)/%.c,$(DEPEND_DIR)/%.d,$(SOURCE_FILES))
MAIN=$(OBJECT_DIR)/$(PROJECT_NAME).o
EXE=$(BIN_DIR)/$(PROJECT_NAME)

TEST_SOURCE_FILES=$(wildcard $(TEST_DIR)/*.c)
TEST_OBJECT_FILES=$(patsubst $(TEST_DIR)/%.c,$(OBJECT_DIR)/%.o,$(TEST_SOURCE_FILES))
TEST_DEPEND_FILES=$(patsubst $(TEST_DIR)/%.c,$(DEPEND_DIR)/%.d,$(TEST_SOURCE_FILES))
TEST_MAIN=$(OBJECT)/$(PROJECT_NAME)-test.o
TEST_EXE=$(BIN_DIR)/$(PROJECT_NAME)-test

# Compiler options

CFLAGS+=-I$(GEN_DIR)
LDFLAGS+=

$(TEST_OBJECT_FILES): CFLAGS+=-I$(SOURCE_DIR)
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
	@# Generate a dependencies file and add $(GEN_DIR) to any $(GEN_FILES) without it
	@$(CC) $(CFLAGS) -MM $< -MT $(patsubst $(DEPEND_DIR)/%.d,$(OBJECT_DIR)/%.o,$@) -MG | \
		sed -r $(patsubst $(GEN_DIR)/%,-e 's@ (%)@ $(GEN_DIR)/\1@g',$(GEN_FILES)) > $@

include $(DEPEND_FILES) $(TEST_DEPEND_FILES)

# Generated files

.SECONDEXPANSION:
$(GEN_FILES): $$(patsubst $(GEN_DIR)/%,$(SOURCE_DIR)/%.in,$$@) Makefile

.PRECIOUS: $(GEN_FILES)
$(GEN_FILES): | $(GEN_DIR)
	@echo Generating $@
	@sed -e 's/@PROJECT_NAME@/$(subst /,\/,$(PROJECT_NAME))/g;' \
		-e 's/@PROJECT_VERSION@/$(subst /,\/,$(PROJECT_VERSION))/g;' \
		-e 's/@PROJECT_BUGREPORT@/$(subst /,\/,$(PROJECT_BUGREPORT))/g;' \
		< $< > $@

# Directories

$(BUILD_DIR) $(OBJECT_DIR) $(GEN_DIR) $(DEPEND_DIR) $(BIN_DIR):
	mkdir -p $@

# Clean

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
