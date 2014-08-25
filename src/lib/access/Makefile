ifndef hyr-access
hyr-access := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

include $(hyr-access)/../../../rules.mk

include $(PROJECT_ROOT)/third_party/Makefile
include $(PROJECT_ROOT)/src/lib/storage/Makefile
include $(PROJECT_ROOT)/src/lib/io/Makefile
include $(PROJECT_ROOT)/src/lib/layouter/Makefile
include $(PROJECT_ROOT)/src/lib/net/Makefile

hyr-access.libname := hyr-access
hyr-access.deps := json hyr-net hyr-storage hyr-io hyr-layouter muparser
hyr-access.libs := boost_regex

ifeq ($(WITH_V8), 1)
V8_RETRIEVAL ?= git clone git://github.com/v8/v8.git $(PROJECT_ROOT)/tools/v8; cd $(PROJECT_ROOT)/tools/v8; git checkout tags/3.27.1; $(MAKE) $(MFLAGS) -C $(PROJECT_ROOT)/tools/v8 dependencies
V8_BUILD ?= $(MAKE) $(MFLAGS) -C $(PROJECT_ROOT)/tools/v8 x64.$(BLD) debuggersupport=on gdbjit=on

$(PROJECT_ROOT)/tools/v8:
	$(V8_RETRIEVAL)

$(PROJECT_ROOT)/tools/v8/out/x64.$(BLD): $(PROJECT_ROOT)/tools/v8
	$(V8_BUILD)

TOOLING += $(PROJECT_ROOT)/tools/v8 $(PROJECT_ROOT)/tools/v8/out/x64.$(BLD)
V8_BASE_DIR := $(PROJECT_ROOT)/tools/v8
V8_BUILD_DIR := $(PROJECT_ROOT)/tools/v8/out/x64.$(BLD)
hyr-access.CPPFLAGS += -DWITH_V8
hyr-access.libs += v8_base.x64 v8_snapshot icui18n icuuc icudata
hyr-access.includes += $(V8_BASE_DIR)/include
hyr-access.LINK_DIRS += $(V8_BUILD_DIR)/obj.target/tools/gyp $(V8_BUILD_DIR)/obj.target/third_party/icu
endif

ifeq ($(WITH_PROFILER), 1)
hyr-access.libs += profiler
endif

$(eval $(call library,hyr-access))
endif
