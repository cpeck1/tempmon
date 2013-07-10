TOPDIR  := $(shell cd ..; pwd)

APP := tempmon
TEST_APP := $(APP)_test
MOCKTEST_APP := $(APP)_mocktest

MTYPE := $(shell uname -m)
LANG := en_US                # for gcc error messages
BUILD_DIR := obj
BUILD_DIR_FULL_PATH := $(CURDIR)/$(BUILD_DIR)
INCLUDE_PATH := . src test

# filenames only - no paths required
COMMON_SRCS := \
	array.c \
	devtypes.c \
	fparse.c \
	usb-operations.c \
	http-operations.c \
	cJSON.c

SRCS := \
	$(COMMON_SRCS) \
	main.c \

TEST_SRCS := \
	$(COMMON_SRCS) \
	array_test.c \
	fparse_test.c \
	test_funcs.c \
	test_main.c

LIBS := -lm -lftdi -lusb-1.0 -lcurl
LIB_PATH := .

MAKEDEPEND := cpp
BUILD_DIR_FULL_PATH := $(CURDIR)/$(BUILD_DIR)
VPATH := $(CURDIR) $(INCLUDE_PATH)

CFLAGS += -c $(foreach inc,$(INCLUDE_PATH),-I$(inc)) -Wall -Wextra -Wno-write-strings
LDFLAGS += $(foreach path,$(LIB_PATH),-L$(path))

ifeq ($(MAKECMDGOALS),test)
	SRCS := $(TEST_SRCS)
	CC := g++
	CXXFLAGS := $(CFLAGS)
	LIBS += -lpthread -lgtest
else 
	CC := gcc
endif

FILES = $(notdir $(SRCS))
OBJS := $(addprefix $(BUILD_DIR)/, $(FILES:.c=.o))
DEPS := $(OBJ:.o=.d)

.PHONY : test clean

all: $(APP)

test: $(TEST_APP)

mocktest: $(MOCKTEST_APP)

$(APP) : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(TEST_APP): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -rf $(BUILD_DIR)/*.[odP] $(APP) $(TEST_APP)

#
# This rule also creates the dependency files
#
$(BUILD_DIR)/%.o : %.c
	$(CC) $(CFLAGS) -MD -o $@ $<
	@cp $(BUILD_DIR)/$*.d $(BUILD_DIR)/$*.P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $(BUILD_DIR)/$*.d >> $(BUILD_DIR)/$*.P; \
	rm -f $(BUILD_DIR)/$*.d

-include $(DEPS)
