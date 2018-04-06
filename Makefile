# Project
PROJECT_NAME=bw
PROJECT_VERSION=1.0
PROJECT_BUGREPORT=<https://github.com/sommd/bw/issues/new>

# Directories
SOURCE_DIR=src
BUILD_DIR=build

$(shell mkdir -p $(BUILD_DIR))

# Compiler options
CFLAGS+=-I$(BUILD_DIR)

# Automatic variables
EXE=$(BUILD_DIR)/$(PROJECT_NAME)
SOURCE_FILES=$(wildcard $(SOURCE_DIR)/*.c)
OBJECT_FILES=$(patsubst $(SOURCE_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCE_FILES))
GENERATED_FILES=$(patsubst $(SOURCE_DIR)/%.in,$(BUILD_DIR)/%,$(wildcard $(SOURCE_DIR)/*.in))
DEPEND_FILES=$(OBJECT_FILES:.o=.deps)

# Executables

all: $(EXE)

$(EXE): $(OBJECT_FILES)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

# Object files

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	$(CC) -c $< $(CFLAGS) -o $@

# Dependencies

.PRECIOUS: $(BUILD_DIR)/%.deps
$(BUILD_DIR)/%.deps: $(SOURCE_DIR)/%.c $(GENERATED_FILES)
	$(CC) $(CFLAGS) -MM $< -MT $(@:.deps=.o) > $@

include $(DEPEND_FILES)

# Generated files

.PRECIOUS: $(BUILD_DIR)/%
$(BUILD_DIR)/%: $(SOURCE_DIR)/%.in Makefile
	sed -e 's/@PROJECT_NAME@/$(subst /,\/,$(PROJECT_NAME))/g;' \
		-e 's/@PROJECT_VERSION@/$(subst /,\/,$(PROJECT_VERSION))/g;' \
		-e 's/@PROJECT_BUGREPORT@/$(subst /,\/,$(PROJECT_BUGREPORT))/g;' \
		< $< > $@

# Clean

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
