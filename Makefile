GENERATED_HEADER := onre.hpp
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

SRCDIR := src
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
SRC_HDRS := $(wildcard src/*.hxx)

.PHONY : all build test gdb clean ftime-trace
all: build $(TARGET)

build : $(GENERATED_HEADER)

test : build $(TARGET)
	$(info RUN     $(TARGET) $(RUNARGS))
	@./$(TARGET) $(RUNARGS)

gdb : build $(TARGET)
	$(info GDB     $(TARGET) $(RUNARGS))
	@gdb --args ./$(TARGET) $(RUNARGS)

clean :
	$(if $(wildcard $(GENERATED_HEADER)), @rm $(GENERATED_HEADER))
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

# make sure changes in the generated umbrella header recompile objects #
$(OBJS): $(GENERATED_HEADER)

$(GENERATED_HEADER) : $(SRC_HDRS)
	@set -eu; \
	{ \
		for hdr in $$(find $(SRCDIR) -maxdepth 1 -name '*.hxx' | sort); do \
			name=$${hdr#$(SRCDIR)/}; \
			printf '__onre_root__ %s\n' "$$name"; \
			awk -v src="$$name" '/^\/\/ === snippet begin ===$$/{exit} /^#include "[^"]+"/{dep=$$0; sub(/^#include "/, "", dep); sub(/"$$/, "", dep); print dep " " src}' "$$hdr"; \
		done; \
	} | tsort | while IFS= read -r hdr; do \
		[ "$$hdr" = "__onre_root__" ] && continue; \
		awk 'BEGIN{emit=0} /^\/\/ === snippet begin ===$$/{emit=1; next} /^\/\/ === snippet end ===$$/{emit=0; next} emit{print}' "$(SRCDIR)/$$hdr"; \
	done | { \
		printf '%s\n%s\n\n' '#ifndef ONRE_REGEX_HPP__' '#define ONRE_REGEX_HPP__'; \
		cat; \
		printf '\n%s\n' '#endif /* #ifndef ONRE_REGEX_HPP__ */'; \
	} > "$@"

# files dependecies #
-include $(DEPS)
