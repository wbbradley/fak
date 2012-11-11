UNAME := $(shell uname)
DEBUG_FLAGS := -DDEBUG -g -O0
NDEBUG_FLAGS := -g -O3

ifeq ($(UNAME),Darwin)
	CPP = clang -std=gnu++0x -DMACOS
	CC = clang -DMACOS
	LINKER = clang -stdlib=libstdc++ -lstdc++
	LINKER_OPTS := $(NDEBUG_FLAGS)
	LINKER_DEBUG_OPTS := $(DEBUG_FLAGS)
else
	CPP = g++ -std=c++0x
	CC = gcc
	LINKER = g++ -stdlib=libc++0x
	LINKER_OPTS := -pthread $(NDEBUG_FLAGS)
	LINKER_DEBUG_OPTS := -pthread $(DEBUG_FLAGS)
endif

BUILD_DIR = build

CFLAGS := \
	-c \
	-Wall \
	-pthread \
	-DDEBUG \
	-g \

STR_REPL_SOURCES = \
				 str-repl.cpp \
				 disk.cpp \
				 mmap_file.cpp \
				 cmd_options.cpp \
				 logger.cpp \
				 utils.cpp \

STR_REPL_OBJECTS = $(addprefix $(BUILD_DIR)/,$(STR_REPL_SOURCES:.cpp=.o))
STR_REPL_TARGET = str-repl

TARGETS = $(STR_REPL_TARGET)

all: build-dir $(TARGETS)

build-dir:
	@if test ! -d $(BUILD_DIR); then mkdir -p $(BUILD_DIR); fi

$(STR_REPL_TARGET): $(STR_REPL_OBJECTS)
	$(LINKER) $(LINKER_OPTS) $(STR_REPL_OBJECTS) -o $(STR_REPL_TARGET)

$(BUILD_DIR)/%.o: %.cpp
	$(CPP) $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

CLEAN = rm -rf $(BUILD_DIR)/*.o $(TARGETS)

clean:
	$(CLEAN)

