# VTUNE Non commercial NR47-P8R6PDS6
# Intel Compiler Linux NRGF-RCXZGDSC

# Suppose the standard position for this file (relative to project root), if not already set
TOP ?=

# Set the project path accordingly
export IMH_PROJECT_PATH := $(shell pwd)$(TOP)

# Set up conservative settings
PRODUCTION ?= 0
USE_GOOGLE_PROFILER ?= 0
USE_V8 ?= 0
MANUAL_PERF_IMPRO ?= 0
COVERAGE_TESTING ?= 0
PAPI_TRACE ?= 0
VERBOSE_BUILD ?= 0
WITH_MYSQL ?= 0
FLTO ?= 0
USE_BACKWARD ?= 0

PLUGINS ?= ccache

PROJECT_INCLUDE ?=
BUILD_FLAGS ?=
CC_BUILD_FLAGS ?=
CXX_BUILD_FLAGS ?=
LINKER_FLAGS ?=
LINKER_DIR ?=
LD_LIBRARY_PATH ?=

# Specify Allocator to use for HYRISE
HYRISE_ALLOCATOR ?=

# Include actual settings, override environment and others
-include $(TOP)settings.mk

COMPILER ?= g++47

include $(TOP)config.$(COMPILER).mk
include $(addprefix mkplugins/,$(addsuffix .mk,$(PLUGINS)))

# Set up settings for mysql tests
HYRISE_MYSQL_HOST ?= 127.0.0.1
HYRISE_MYSQL_PORT ?= 3306
HYRISE_MYSQL_USER ?= root
HYRISE_MYSQL_PASS ?= root

OSTYPE = $(shell uname | tr '[A-Z]' '[a-z]')
export build_dir ?= build

# OS switches
ifneq (,$(findstring linux,$(OSTYPE)))
	LIB_EXTENSION := so
	BUILD_FLAGS += -fPIC -D WITH_NUMA
	LINKER_FLAGS += -lnuma -ldl -Wl,-no-as-needed
	SHARED_LIB := -shared
else
	BUILD_FLAGS += -D NO_PREFETCHING
	LIB_EXTENSION := dylib
	SHARED_LIB := -dynamiclib
endif

BUILD_FLAGS += -pipe -march=native -ggdb -Wall -Wextra -Wno-unused-parameter
CXX_BUILD_FLAGS += --std=c++0x
LINKER_FLAGS +=

ifeq ($(PRODUCTION), 1)
	BUILD_FLAGS += -O3 -fbranch-target-load-optimize -frerun-cse-after-loop 
else
	BUILD_FLAGS += -O0  -fno-inline -g3 -ggdb
endif

# Specify the allocator using a linker flag
ifneq ($(HYRISE_ALLOCATOR),)
	LINKER_FLAGS += -l$(HYRISE_ALLOCATOR)
endif

ifeq ($(FLTO), 1)
	BUILD_FLAGS += -flto -fwhole-program
	LINKER_FLAGS += -flto
endif

ifeq ($(PAPI_TRACE), 1)
	BUILD_FLAGS += 
	LINKER_FLAGS += -lpapi
endif

ifeq ($(USE_GOOGLE_PROFILER), 1)
	BUILD_FLAGS += -D HYRISE_USE_GOOGLE_PROFILER
	LINKER_FLAGS += -lprofiler
endif

ifeq ($(COVERAGE_TESTING), 1)
	BUILD_FLAGS += -fprofile-arcs -ftest-coverage
	LINKER_FLAGS += -lgcov
endif

ifeq ($(WITH_MYSQL), 1)
	BUILD_FLAGS += -D WITH_MYSQL
	LINKER_FLAGS += -lmysqlclient
endif

ifeq ($(USE_BACKWARD), 1)
	BUILD_FLAGS += -D BACKWARD_HAS_BFD -D USE_BACKWARD
	LINKER_FLAGS += -lbfd
endif

ifeq ($(USE_V8), 1)
	BUILD_FLAGS += -D WITH_V8
	LINKER_FLAGS += -lv8
	PROJECT_INCLUDE += $(V8_BASE_DIRECTORY)/include
	ifeq ($(PRODUCTION), 1)
		LINKER_DIR += $(V8_BASE_DIRECTORY)/out/x64.release/obj.target/tools/gyp
	else
		LINKER_DIR += $(V8_BASE_DIRECTORY)/out/x64.debug/obj.target/tools/gyp
	endif
endif

JSON_PATH	:=	$(IMH_PROJECT_PATH)/third_party/jsoncpp
FTPRINTER_PATH	:=	$(IMH_PROJECT_PATH)/third_party/ftprinter/include
CEREAL_PATH := $(IMH_PROJECT_PATH)/third_party/cereal/include
PROJECT_INCLUDE += $(IMH_PROJECT_PATH)/src/lib $(IMH_PROJECT_PATH)/third_party $(FTPRINTER_PATH) $(JSON_PATH) $(CEREAL_PATH)
LINKER_FLAGS += -llog4cxx -lpthread -lboost_system
BINARY_LINKER_FLAGS += -lbackward-hyr

BUILD_DIR = $(IMH_PROJECT_PATH)/$(build_dir)/

# Settings for the correct includes
LINKER_DIR += $(BUILD_DIR) /usr/local/lib /usr/lib64
LD_LIBRARY_PATH := $(LD_LIBRARY_PATH):$(BUILD_DIR):/usr/local/lib:/usr/lib64

CC_BUILD_FLAGS += $(BUILD_FLAGS)
CXX_BUILD_FLAGS += $(BUILD_FLAGS)

export
