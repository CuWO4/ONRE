TARGET_NAME := test
RUNARGS ?=

EXT := cpp
CXX := clang++
CXXFLAGS := -Wall -std=c++20 -ftemplate-depth=65536 -fbracket-depth=65536 -fconstexpr-steps=4294967295 -fconstexpr-depth=65536
LDFLAGS :=
LDLIBS :=

ifeq ($(MAKECMDGOALS),gdb)
	CXXFLAGS += -O0 -g
	LDFLAGS += -g
	MODE_DIR := gdb_build
else
	CXXFLAGS += -O2
	MODE_DIR := release_build
endif

ifeq ($(MAKECMDGOALS),ftime-trace)
	CXXFLAGS += -ftime-trace
endif

ifeq ($(OS),Windows_NT)
	OS_SUFFIX := -win_nt
else
	OS_SUFFIX := -$(shell uname -r)
endif

# set -MMD -MP to generate dependent files #
CXXFLAGS += -MMD -MP

TMPDIR_BASE := tmp
DEBUGDIR_BASE := debug

# store .o and .d files #
TMPDIR := $(TMPDIR_BASE)/$(MODE_DIR)$(OS_SUFFIX)
# store the target file #
DEBUGDIR := $(DEBUGDIR_BASE)/$(MODE_DIR)$(OS_SUFFIX)

ifeq ($(OS),Windows_NT)
	TARGET := $(DEBUGDIR)/$(TARGET_NAME).exe
else
	TARGET := $(DEBUGDIR)/$(TARGET_NAME)
endif

# recursive wildcard #
rwildcard = $(foreach d,$(wildcard $1/*),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# cross-platform mkdir -p equivalent #
ifeq ($(OS),Windows_NT)
	MKDIR_P = if not exist "$(1)" mkdir "$(1)"
else
	MKDIR_P = mkdir -p "$(1)"
endif

# sources, objects and dependencies #
SRCS := $(call rwildcard,.,*.$(EXT))
OBJS := $(patsubst %.$(EXT),$(TMPDIR)/%.o,$(SRCS))
DEPS := $(patsubst %.$(EXT),$(TMPDIR)/%.d,$(SRCS))

.PHONY : all test gdb clean ftime-trace
all: $(TARGET)

test : $(TARGET)
	$(info RUN     $(TARGET) $(RUNARGS))
	@./$(TARGET) $(RUNARGS)

gdb : $(TARGET)
	$(info GDB     $(TARGET) $(RUNARGS))
	@gdb --args ./$(TARGET) $(RUNARGS)

clean :
	$(if $(wildcard $(DEBUGDIR_BASE)), @rm -r $(DEBUGDIR_BASE))
	$(if $(wildcard $(TMPDIR_BASE)), @rm -r $(TMPDIR_BASE))

ftime-trace :
	$(MAKE) clean
	$(MAKE) all

# link #
$(TARGET) : $(OBJS)
	@$(call MKDIR_P,$(dir $@))
	$(info LD      $@)
	@$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

# compile #
$(TMPDIR)/%.o : %.$(EXT)
	@$(call MKDIR_P,$(dir $@))
	$(info CXX     $<)
	@$(CXX) -c $(CXXFLAGS) -o $@ $<

# files dependecies #
-include $(DEPS)
