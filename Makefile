CC = gcc
LIBNEMI_TARGET = libnemi.so
LOADER_TARGET  = nemi

LIBVTERM_DIR = ./libs/libvterm-0.3.3


CC_FLAGS = -ggdb -Wall -Wextra -I$(LIBVTERM_DIR)/include 
LD_FLAGS = -lm -Wl,-rpath=$(LIBVTERM_DIR) -L$(LIBVTERM_DIR) -lvterm_l



LIBNEMI_SRC_DIR = ./src
LOADER_SRC_DIR  = ./loader
OBJ_DIR         = ./obj

GRAPHICS ?= opengl

ifeq ($(GRAPHICS), opengl)
	CC_FLAGS += -DGRAPHICS_OPENGL
	CC_FLAGS += -I/usr/include/freetype2
	CC_FLAGS += -I/usr/include/libpng16
	LD_FLAGS += -lglfw
	LD_FLAGS += -lGLEW
	LD_FLAGS += -lfreetype
endif

LIBNEMI_SRC_FILES = $(shell find $(LIBNEMI_SRC_DIR) -type f -name '*.c')
LIBNEMI_OBJ_FILES = $(patsubst $(LIBNEMI_SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(LIBNEMI_SRC_FILES))


LOADER_SRC_FILES = $(shell find $(LOADER_SRC_DIR) -type f -name '*.c')
LOADER_OBJ_FILES = $(patsubst $(LOADER_SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(LOADER_SRC_FILES))



all: pre_build $(LIBNEMI_TARGET) $(LOADER_TARGET)


pre_build:
ifneq ($(GRAPHICS), opengl)
	@echo "Graphics backend options are:"
	@echo "'opengl'     : Uses GLFW, GLEW and Freetype2"
	exit 1
endif

# Compile and link libnemi.

$(LIBNEMI_OBJ_FILES): $(OBJ_DIR)/%.o: $(LIBNEMI_SRC_DIR)/%.c
	@mkdir -p $(shell dirname $@)
	@echo "($(LIBNEMI_TARGET)): $<"
	@$(CC) $(CC_FLAGS) -fPIC -c $< -o $@ || (echo -e "\033[1;31mFailed to compile $<\033[0m")

$(LIBNEMI_TARGET): $(LIBNEMI_OBJ_FILES)
	@$(CC) $(LIBNEMI_OBJ_FILES) $(LD_FLAGS) -shared -o $@


# Compile and link loader for libnemi.

$(LOADER_OBJ_FILES): $(OBJ_DIR)/%.o: $(LOADER_SRC_DIR)/%.c
	@echo "($(LOADER_TARGET)): $<"
	@$(CC) $(CC_FLAGS) -c $< -o $@

$(LOADER_TARGET): $(LOADER_OBJ_FILES)
	@$(CC) $(LD_FLAGS) $(LOADER_OBJ_FILES) $(OBJ_DIR)/nmt_string.o $(OBJ_DIR)/memory.o -o $@


clean:
	rm -rfv $(OBJ_DIR)
	rm -v $(LIBNEMI_TARGET)
	rm -v $(LOADER_TARGET)


.PHONY: all clean

