TARGET := test.exe
EXTERN := cpp
COMPILER := clang++-18

COMPILE_OPTION := -Wall -O2 --std=c++23 -fbracket-depth=65536 -ftemplate-depth=65536
# to generate dependent files #
COMPILE_OPTION_DES := -MMD -MP 

# store .o and .d files #
TMPDIR := tmp
# store the target file #
DEBUGDIR := debug

# sources, objects and dependencies #
SRCS := $(wildcard tests/*.$(EXTERN))
OBJS := $(patsubst tests/%.$(EXTERN), $(TMPDIR)/%.o, $(SRCS))
DEPS := $(patsubst tests/%.$(EXTERN), $(TMPDIR)/%.d, $(SRCS))

# link #
$(DEBUGDIR)/$(TARGET) : $(OBJS) | $(DEBUGDIR)
	@ echo linking...
	$(COMPILER) $(OBJS) -o $(DEBUGDIR)/$(TARGET)
	@ echo completed!

# compile #
$(TMPDIR)/%.o : tests/%.$(EXTERN) | $(TMPDIR)
	@ echo compiling $<...
	$(COMPILER) $< -o $@ -c $(COMPILE_OPTION) $(COMPILE_OPTION_DES)

# create TMPDIR when it does not exist #
$(TMPDIR) :
	@ mkdir $(patsubst ./%, %, $(TMPDIR))

$(DEBUGDIR) :
	@ mkdir $(DEBUGDIR)

# files dependecies #
-include $(DEPS)

# test command #
.PHONY : test
test : $(DEBUGDIR)/$(TARGET)
	./$(DEBUGDIR)/$(TARGET)

# clean command #
.PHONY : clean
clean :
	@ echo try to clean...
	rm -r $(DEBUGDIR) $(TMPDIR)
	@ echo completed!