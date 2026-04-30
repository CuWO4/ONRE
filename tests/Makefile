# requires: TARGET_HEADER (absolute path) CXX (default "clang++")
# OPTIMIZE_LEVEL (default "-O2") EXTRA_CXX_FLAGS (default "") EXTRA_LD_FLAGS (default "")
# EXECUTOR (default "") EXECUTOR_ARGS (default "") RUNARGS (default "")

ifeq ($(TARGET_HEADER),)
$(error TARGET_HEADER is not set. Please provide absolute path to onre.hpp)
endif

CXX ?= clang++
OPTIMIZE_LEVEL ?= -O2
EXTRA_CXX_FLAGS ?=
EXTRA_LD_FLAGS ?=
EXECUTOR ?=
EXECUTOR_ARGS ?=
RUNARGS ?=

-include ../define.mk

TARGET_NAME := test

EXT := cpp
# some features rely on macro switch, therefore use
#   #define ONRE_DOTALL
#   #include ONRE_HPP_FILE
# and inject ONRE_HPP_FILE=/path/to/onre.hpp instead of -include /path/to/onre.hpp
CXXFLAGS := -Wall -std=c++20 -DONRE_HPP_FILE='"$(TARGET_HEADER)"' $(OPTIMIZE_LEVEL) $(EXTRA_CXX_FLAGS)
LDFLAGS := $(EXTRA_LD_FLAGS)
LDLIBS :=

ifeq ($(CXX),clang++)
	CXXFLAGS += -ftemplate-depth=65536 -fbracket-depth=65536 -fconstexpr-steps=4294967295 -fconstexpr-depth=65536
	CXXFLAGS += -stdlib=libc++
	LDFLAGS += -stdlib=libc++
else # g++
	CXXFLAGS += -ftemplate-depth=65536 -fconstexpr-depth=65536
endif

ifeq ($(OS),Windows_NT)
	OS_SUFFIX := win_nt
else
	OS_SUFFIX := $(shell uname -r)
endif
MODE_DIR := $(CXX)-$(EXECUTOR)-$(OS_SUFFIX)

# set -MMD -MP to generate dependent files #
CXXFLAGS += -MMD -MP

TMPDIR := $(TMPDIR_BASE)/$(MODE_DIR)
DEBUGDIR := $(DEBUGDIR_BASE)/$(MODE_DIR)

ifeq ($(OS),Windows_NT)
	TARGET := $(DEBUGDIR)/$(TARGET_NAME).exe
else
	TARGET := $(DEBUGDIR)/$(TARGET_NAME)
endif

# sources, objects and dependencies #
SRCS := $(call rwildcard,.,*.$(EXT))
OBJS := $(patsubst %.$(EXT),$(TMPDIR)/%.o,$(SRCS))
DEPS := $(patsubst %.$(EXT),$(TMPDIR)/%.d,$(SRCS))

ifeq ($(CXX),clang++)
	CXX_INFO = CLANG++ $<
else
	CXX_INFO = G++     $<
endif

.PHONY : all run
all: $(TARGET)

run : $(TARGET)
	$(info RUN     $(EXECUTOR) $(EXECUTOR_ARGS) $(TARGET) $(RUNARGS))
	@$(EXECUTOR) $(EXECUTOR_ARGS) $(TARGET) $(RUNARGS)

# link #
$(TARGET) : $(OBJS)
	@$(call MKDIR_P,$(dir $@))
	$(info LD      $@)
	@$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

# compile #
$(TMPDIR)/%.o : %.$(EXT)
	@$(call MKDIR_P,$(dir $@))
	$(info $(CXX_INFO))
	@$(CXX) -c $(CXXFLAGS) -o $@ $<

# files dependecies #
-include $(DEPS)
