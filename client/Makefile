TOPDIR  := $(shell cd ..; pwd)

APP := tempmon
TEST_APP := $(APP)_test

MTYPE := $(shell uname -m)
LANG := en_US                # for gcc error messages
BUILD_DIR := obj

# filenames only - no paths required
COMMON_SRCS := \
	src/array.c \
	src/devtypes.c \
	src/fparse.c \
	src/usb-operations.c

SRCS := \
	$(COMMON_SRCS) \
	src/main.c

TEST_SRCS := \
	$(COMMON_SRCS) \
	test/array_test.c \
	test/fparse_test.c \
	test/test_funcs.c \
	test/test_main.c

LIBS := -lm -lftdi -lusb-1.0
LIB_PATH := .

ifeq ($(MAKECMDGOALS),test)
  SRCS := $(TEST_SRCS)
  CC := g++
  CXXFLAGS := $(CFLAGS)
  LIBS += -lpthread -lgtest
else
  CC := gcc
endif

FILES = $(notdir $(SRCS))
PATHS = $(sort $(dir $(SRCS) ) )
OBJS := $(addprefix $(BUILD_DIR)/, $(FILES:.c=.o))
DEPS := $(OBJ:.o=.d)

INCLUDE_PATH := $(foreach inc,$(PATHS),$(inc))
VPATH := $(CURDIR) $(INCLUDE_PATH)

CFLAGS += -c $(foreach inc,$(INCLUDE_PATH),-I$(inc)) -Wall -Wextra -Wno-write-strings
LDFLAGS += $(foreach path,$(LIB_PATH),-L$(path))

ifndef VERBOSE
  SILENT := @
endif

.PHONY : test clean

all: $(APP)

test: $(TEST_APP)

$(APP) : $(OBJS)
	@echo "linking $@"
	$(SILENT)$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(TEST_APP): $(OBJS)
	@echo "linking $@"
	$(SILENT)$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -rf $(BUILD_DIR)/*.[odP] $(APP) $(TEST_APP)

#
# This rule also creates the dependency files
#
$(BUILD_DIR)/%.o : %.c
	@echo "compiling $<..."
	$(SILENT)$(CC) $(CFLAGS) -MD -o $@ $<
	$(SILENT)cp $(BUILD_DIR)/$*.d $(BUILD_DIR)/$*.P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $(BUILD_DIR)/$*.d >> $(BUILD_DIR)/$*.P; \
	rm -f $(BUILD_DIR)/$*.d

-include $(DEPS)
