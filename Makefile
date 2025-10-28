.PHONY: all build debug release clean valgrind install uninstall

SRC_DIR := src
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
DEP_DIR := $(BUILD_DIR)/dep
INCLUDE_DIR := include
TEST_DIR := tests

PROJECT_NAME := rose
VERSION := 0.1.0
TARGET := $(BUILD_DIR)/$(PROJECT_NAME)_$(VERSION)

CC := gcc
CFLAGS := -Wall -Wextra -I$(INCLUDE_DIR) # -std=c99 
LDFLAGS := -lm # -lgmp -lmpfr

MODE ?= debug
ifeq ($(MODE),debug)
	CFLAGS += -O0 -ggdb
	BUILD_TYPE := Debug
else ifeq ($(MODE),release)
	CFLAGS += -O3 -DNDEBUG -flto -march=native
	BUILD_TYPE := Release
endif

SRC := \
    $(SRC_DIR)/main.c \
	$(SRC_DIR)/token.c \
    $(SRC_DIR)/lexer.c \
	$(SRC_DIR)/node.c \
	$(SRC_DIR)/parser.c \
	$(SRC_DIR)/sema.c \
	$(SRC_DIR)/value.c \
	$(SRC_DIR)/env.c \
	$(SRC_DIR)/stack.c \
	$(SRC_DIR)/eval.c

OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEP := $(OBJ:$(OBJ_DIR)/%.o=$(DEP_DIR)/%.d)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@) $(DEP_DIR)
	$(CC) $(CFLAGS) -MMD -MP -MF $(DEP_DIR)/$(notdir $(basename $@)).d -c $< -o $@
# 	@mv $(basename $@).d $(DEP_DIR)/$(notdir $(basename $@)).d

-include $(DEP)

debug release:
	$(MAKE) MODE=$@ all

clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)

VALGRIND_OPTS := \
	--tool=memcheck \
	--leak-check=full \
	--show-leak-kinds=all \
	--track-origins=yes \
	--verbose \
	--errors-for-leak-kinds=all \
	--undef-value-errors=yes \
	--num-callers=40

valgrind: $(TARGET)
	valgrind $(VALGRIND_OPTS) $(TARGET) example/valgrind.txt

PREFIX ?= /usr/local
BINDIR := $(PREFIX)/bin

install: $(TARGET)
	@echo "Installing $(PROJECT_NAME) to $(BINDIR)..."
	@mkdir -p $(BINDIR)
	@cp $(TARGET) $(BINDIR)/$(PROJECT_NAME)
	@chmod +x $(BINDIR)/$(PROJECT_NAME)
	@echo "Done."

uninstall:
	@echo "Removing $(PROJECT_NAME) from $(BINDIR)..."
	@rm -f $(BINDIR)/$(PROJECT_NAME)
	@echo "Done."