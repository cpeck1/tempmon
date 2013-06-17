TOPDIR  := $(shell cd ..; pwd)

APP := tempmon
TEST_APP := $(APP)_test

MTYPE := $(shell uname -m)
LANG := en_US                # for gcc error messages
BUILD_DIR := obj
BUILD_DIR_FULL_PATH := $(CURDIR)/$(BUILD_DIR)
INCLUDE_PATH := . src test

# filenames only - no paths required
SRCS := \
	array.c \
	devtypes.c \
	fparse.c \
	main.c \
	usb-operations.c

TEST_SRCS := \
	array_t.c \
	fparse_t.c \
	usb-operations.c \
	test_func.c

LIBS := -lm -lftdi -lusb-1.0
LIB_PATH := .

CC := gcc
CXX := $(CC)
MAKEDEPEND := cpp
BUILD_DIR_FULL_PATH := $(CURDIR)/$(BUILD_DIR)
VPATH := $(CURDIR) $(INCLUDE_PATH) $(BUILD_DIR)
OBJS := $(addsuffix .o, $(basename $(notdir $(SRCS))))
OBJS_RELATIVE_PATH := $(addprefix $(BUILD_DIR)/, $(OBJS))

DEPS := $(addsuffix .P, $(basename $(notdir $(SRC))))
DEPS_FULL_PATH := $(addprefix $(CURDIR)/$(BUILD_DIR)/, $(DEPS))

CFLAGS += -c $(foreach inc,$(INCLUDE_PATH),-I$(inc)) -Wall -Wextra -Wno-write-strings
LDFLAGS += $(foreach path,$(LIB_PATH),-L$(path))

all: $(APP)

$(APP) : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS_RELATIVE_PATH) $(LIBS)

clean:
	rm -rf $(BUILD_DIR)/*.[odP] $(APP)

#
# This rule also creates the dependency files
#
%.o : %.c
	$(COMPILE.c) -MD -o $(BUILD_DIR)/$@ $<
	@cp $(BUILD_DIR)/$*.d $(BUILD_DIR)/$*.P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $(BUILD_DIR)/$*.d >> $(BUILD_DIR)/$*.P; \
	rm -f $(BUILD_DIR)/$*.d

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS_FULL_PATH)
endif
