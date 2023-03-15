# Makefile for TCE
# Created by t.lemercier on 2023-03-15

CXX = g++

OBJECT_DIR = ../object
SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, $(OBJECT_DIR)/%.o, $(SRCS))

MANDATORY_FLAG = -Wall -Wcast-qual -fno-exceptions -std=c++20 -pedantic -Wextra -Wshadow -Wmissing-declarations -Wunused
OPTI_FLAG = -O3 -march=native
DEBUG_FLAG = -DDEBUG
RELEASE_NAME = tce.exe

all: $(RELEASE_NAME)

$(RELEASE_NAME): $(OBJS)
	@echo Linking $@
	@$(CXX) $(OPTI_FLAG) $(MANDATORY_FLAG) -o ../$@ $^
	@echo Done.

$(OBJECT_DIR):
	@mkdir $(OBJECT_DIR)

$(OBJECT_DIR)/%.o: %.cpp | $(OBJECT_DIR)
	@echo Compiling $<
	@$(CXX) $(OPTI_FLAG) $(MANDATORY_FLAG) -c $< -o $@

run: $(RELEASE_NAME)
	@echo Running
	@../$(RELEASE_NAME)

clean:
	@echo Cleaning
	@rm -f $(OBJECT_DIR)/*o
	@rmdir $(OBJECT_DIR)
	@rm -f ../$(RELEASE_NAME)
	@echo Done.

.PHONY: clean run
