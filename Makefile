-include define.mk

TESTARGS ?=

TARGET_HEADER_NAME := onre.hpp

TARGET_HEADER := $(BASE_DIR)/$(TARGET_HEADER_NAME)
DEBUG_TARGET_HEADER := $(TMPDIR_BASE)/include/$(TARGET_HEADER_NAME)

.PHONY : build test test-ftime-trace test-gdb test-callgrind clean
build : $(TARGET_HEADER)

test : $(DEBUG_TARGET_HEADER)
	@echo 'TEST    CLANG++'
	@time $(MAKE) -C tests run \
	 	TARGET_HEADER=$(DEBUG_TARGET_HEADER) \
	 	CXX=clang++ \
	 	RUNARGS='$(TESTARGS)' \
		--no-print-directory
	@echo 'TEST    G++'
	@time $(MAKE) -C tests run \
		TARGET_HEADER=$(DEBUG_TARGET_HEADER) \
		CXX=g++ \
		RUNARGS='$(TESTARGS)' \
		--no-print-directory

test-ftime-trace : $(DEBUG_TARGET_HEADER)
	@time $(MAKE) -C tests all \
	 	TARGET_HEADER=$(DEBUG_TARGET_HEADER) \
	 	CXX=clang++ \
		EXTRA_CXX_FLAGS=-ftime-trace \
		--no-print-directory

test-gdb : $(DEBUG_TARGET_HEADER)
	@$(MAKE) -C tests run \
	 	TARGET_HEADER=$(DEBUG_TARGET_HEADER) \
	 	CXX=clang++ \
		OPTIMIZE_LEVEL=-O0 \
		EXTRA_CXX_FLAGS=-g \
		EXTRA_LD_FLAGS=-g \
		EXECUTOR=gdb \
		EXECUTOR_ARGS=--args \
		RUNARGS=$(TESTARGS) \
		--no-print-directory

test-callgrind : $(DEBUG_TARGET_HEADER)
	@$(MAKE) -C tests run \
	 	TARGET_HEADER=$(DEBUG_TARGET_HEADER) \
	 	CXX=clang++ \
		OPTIMIZE_LEVEL=-O2 \
		EXTRA_CXX_FLAGS='-g -gdwarf-4' \
		EXTRA_LD_FLAGS=-g \
		EXECUTOR=valgrind \
		EXECUTOR_ARGS='--tool=callgrind --cache-sim=yes --branch-sim=yes --callgrind-out-file=$(BASE_DIR)/callgrind.out' \
		RUNARGS=$(TESTARGS) \
		--no-print-directory

clean :
	$(if $(wildcard $(TARGET_HEADER)), @rm $(TARGET_HEADER))
	$(if $(wildcard $(DEBUGDIR_BASE)), @rm -r $(DEBUGDIR_BASE))
	$(if $(wildcard $(TMPDIR_BASE)), @rm -r $(TMPDIR_BASE))

-include build.mk
