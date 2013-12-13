hyr-access := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../../rules.mk

include $(PROJECT_ROOT)/third_party/Makefile
include $(PROJECT_ROOT)/src/lib/storage/Makefile
include $(PROJECT_ROOT)/src/lib/io/Makefile
include $(PROJECT_ROOT)/src/lib/layouter/Makefile
include $(PROJECT_ROOT)/src/lib/net/Makefile

hyr-access.libname := hyr-access
hyr-access.deps := json hyr-net hyr-storage hyr-io hyr-layouter
hyr-access.libs := boost_regex

ifeq ($(USE_V8), 1)
hyr-access.CFLAGS += -DWITH_V8
hyr-access.LIBS += -lv8
hyr-access.includes += $(V8_BASE_DIRECTORY)/include
hyr-access.LINK_DIRS += $(V8_BASE_DIRECTORY)/out/x64.$(BLD)/obj.target/tools/gyp
endif

ifeq ($(WITH_PROFILER), 1)
hyr-access.libs += profiler
endif

$(eval $(call library,hyr-access))
