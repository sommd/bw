# Project
PROJECT_NAME=bw
PROJECT_VERSION=1.0
PROJECT_BUGREPORT=<https://github.com/sommd/bw/issues/new>

# Directories
SOURCE_DIR=src
BUILD_DIR=build

OBJECT_DIR=$(BUILD_DIR)/obj
GEN_DIR=$(BUILD_DIR)/gen
DEPEND_DIR=$(BUILD_DIR)/dep
BIN_DIR=$(BUILD_DIR)/bin

# Compiler options
CFLAGS+=-I$(GEN_DIR)

# Automatic variables
SOURCE_FILES=$(wildcard $(SOURCE_DIR)/*.c)
OBJECT_FILES=$(patsubst $(SOURCE_DIR)/%.c,$(OBJECT_DIR)/%.o,$(SOURCE_FILES))
GEN_FILES=$(patsubst $(SOURCE_DIR)/%.in,$(GEN_DIR)/%,$(wildcard $(SOURCE_DIR)/*.in))
DEPEND_FILES=$(patsubst $(SOURCE_DIR)/%.c,$(DEPEND_DIR)/%.d,$(SOURCE_FILES))
EXE=$(BIN_DIR)/$(PROJECT_NAME)

# Executables

all: $(EXE)

debug: CFLAGS:=$(CFLAGS) -g
debug: $(EXE)

$(EXE): $(OBJECT_FILES) | $(BIN_DIR)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

# Object files

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c | $(OBJECT_DIR)
	$(CC) -c $< $(CFLAGS) -o $@

# Dependencies

.PRECIOUS: $(DEPEND_DIR)/%.d
$(DEPEND_DIR)/%.d: $(SOURCE_DIR)/%.c | $(GEN_FILES) $(DEPEND_DIR)
	$(CC) $(CFLAGS) -MM $< -MT $(patsubst $(SOURCE_DIR)/%.c,$(OBJECT_DIR)/%.o,$<) -MF $@

include $(DEPEND_FILES)

# Generated files

.PRECIOUS: $(GEN_DIR)/%
$(GEN_DIR)/%: $(SOURCE_DIR)/%.in Makefile | $(GEN_DIR)
	sed -e 's/@PROJECT_NAME@/$(subst /,\/,$(PROJECT_NAME))/g;' \
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
