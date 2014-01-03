ifndef RULES_MK
RULES_MK := defined

RUNDIR := $(shell pwd)
ifndef TOP
TOP := $(shell \
top=$(RUNDIR); \
while [ ! -r "$$top/.top" ] && [ "$$top" != "" ]; do \
top=$${top%/*}; \
done; \
echo $$top)
endif


define uniq_
  $(eval seen :=)
  $(foreach _,$1,$(if $(filter $_,${seen}),,$(eval seen += $_)))
  ${seen}
endef

reverse = $(if $(1),$(call reverse,$(wordlist 2,$(words $(1)),$(1)))) $(firstword $(1))
uniq = $(strip $(call reverse,$(call uniq_,$(call reverse,$(1)))))
inherit = $(foreach _,$(addsuffix .$(2),$(1)),$(value $(_)))

define inherit_
$(1).$(2) := $$(call uniq, $$($(1).$(2)) $$(call inherit,$$($(1).deps),$(2)))
endef

INHERITABLES := deps LINK_DIRS INCLUDE_DIRS LIBS LDFLAGS CPPFLAGS CFLAGS CXXFLAGS OBJS EXT_LIBS
inherit_all = $(foreach _,$(INHERITABLES),$$(eval $$(call inherit_,$(1),use_$(_))))

ifdef build_debug
echo_cmd =
else
echo_cmd = @echo "$(subst $(PROJECT_ROOT)/,,$(1))";
endif

define library
# user replace-able
ifndef $(1).defined
$(1).deps ?=
$(1).cpps ?= $$(shell find $($(1)) -type f -name "*.cpp")
$(1).objs ?= $$($(1).cpps:%=$(OBJDIR)%.o)
$(1).lib ?= $(RESULT_DIR)/lib$$($(1).libname)_$(BLD).a
$(1).libs ?=
$(1).includes ?=
$(1).defined := is defined
$(1).LDFLAGS ?=
$(1).CPPFLAGS ?=
$(1).CFLAGS ?=
$(1).CXXFLAGS ?=

# rules
$$($(1).lib) : $$($(1).objs)
all += $$($(1).lib)
# inheriting information by first filling in our pieces...
$(1).use_deps := $$($(1).lib)
$(1).use_LIBS := $$($(1).libname)_$(BLD) $$($(1).libs)
$(1).use_EXT_LIBS := $$($(1).libs)
$(1).use_LINK_DIRS := $$(realpath $$(dir $$($(1).lib)))
$(1).use_INCLUDE_DIRS := $$($(1).includes)
$(1).use_LDFLAGS := $$($(1).LDFLAGS)
$(1).use_CPPFLAGS := $$($(1).CPPFLAGS)
$(1).use_CFLAGS := $$($(1).CFLAGS)
$(1).use_CXXFLAGS := $$($(1).CXXFLAGS)
$(1).use_OBJS := $$($(1).objs)
$$(eval $$(call inherit_all,$(1)))

$$($(1).objs) : INCLUDE_DIRS += $$($(1).use_INCLUDE_DIRS)
$$($(1).objs) : CPPFLAGS += $$($(1).use_CPPFLAGS)
$$($(1).objs) : CFLAGS += $$($(1).use_CFLAGS)
$$($(1).objs) : CXXFLAGS += $$($(1).use_CXXFLAGS)
$$($(1).objs) : LIB := $(1)
ifdef build_debug
$$(info loaded $(1) with deps $$($(1).deps))
endif
ifneq "$(MAKECMDGOALS)" "clean"
-include $$($(1).cpps:%=$$(OBJDIR)%.d)
endif
endif
endef

define binary
# user replace-able
ifndef $(1).defined
$(1).deps ?=
$(1).cpps ?= $$(wildcard $($(1))/*.cpp)
$(1).objs ?= $$($(1).cpps:%=$(OBJDIR)%.o)
$(1).binary ?= $(RESULT_DIR)/$($(1).binname)_$(BLD)
$(1).defined := is defined
$(1).libs ?=
# rules
$(1).use_OBJS := $$($(1).objs)
$$(eval $$(call inherit_all,$(1)))

$$($(1).binary) : $$($(1).objs) $$($(1).use_deps)
all: $$($(1).binary)
all += $$($(1).binary)

$$($(1).binary) : LDFLAGS += $$($(1).use_LDFLAGS)
$$($(1).binary) : LINK_DIRS += $$($(1).use_LINK_DIRS)
$$($(1).binary) : LIBS += $$($(1).use_LIBS) $$($(1).libs)
$$($(1).objs) : INCLUDE_DIRS += $$($(1).use_INCLUDE_DIRS)
$$($(1).objs) : CXXFLAGS += $$($(1).use_CXXFLAGS)
$$($(1).objs) : CFLAGS += $$($(1).use_CFLAGS)
$$($(1).objs) : CPPFLAGS += $$($(1).use_CPPFLAGS)

ifdef build_debug
$$(info loaded $(1) with deps $$($(1).deps))
endif
ifneq "$(MAKECMDGOALS)" "clean"
-include $$($(1).cpps:%=$$(OBJDIR)%.d)
endif
endif
endef

define full_link_binary
# user replace-able
ifndef $(1).defined
$(1).deps ?=
$(1).cpps ?= $$(wildcard $($(1))/*.cpp)
$(1).objs ?= $$($(1).cpps:%=$(OBJDIR)%.o)
$(1).binary ?= $(RESULT_DIR)/$($(1).binname)_$(BLD)
$(1).defined := is defined
$(1).libs ?=
# rules
$(1).use_OBJS := $$($(1).objs)
$$(eval $$(call inherit_all,$(1)))

$$($(1).binary) : $$($(1).use_OBJS)
all: $$($(1).binary)
all += $$($(1).binary)

$$($(1).binary) : LDFLAGS += $$($(1).use_LDFLAGS)
$$($(1).binary) : LINK_DIRS += $$($(1).use_LINK_DIRS)
$$($(1).binary) : LIBS += $$($(1).use_EXT_LIBS) $$($(1).libs)
$$($(1).objs) : INCLUDE_DIRS += $$($(1).use_INCLUDE_DIRS)
$$($(1).objs) : CXXFLAGS += $$($(1).use_CXXFLAGS)
$$($(1).objs) : CFLAGS += $$($(1).use_CFLAGS)
$$($(1).objs) : CPPFLAGS += $$($(1).use_CPPFLAGS)

ifdef build_debug
$$(info loaded $(1) with deps $$($(1).deps))
endif
ifneq "$(MAKECMDGOALS)" "clean"
-include $$($(1).cpps:%=$$(OBJDIR)%.d)
endif
endif
endef

define test-binary
$(eval $(call binary,$(1)))
test-tgts += test_$$($(1).binary)
.PHONY: test_$$($(1).binary)
test_$$($(1).binary): $$($(1).binary)
	$$(TESTPREFIX) $$($(1).binary) $$(TESTPARAM)
endef

### PROJECT SPECIFIC STUFF ###

PROJECT_ROOT := $(TOP)
OSNAME := $(shell uname -s)

%.mk: makefiles/%.default.mk
	@[ -e $@ ] || echo "Grabbing default $@"; cp $< $@
	@touch $@

CFLAGS :=
CPPFLAGS :=
CXXFLAGS :=
COMMON_FLAGS :=
LDFLAGS :=
LIBS := log4cxx
LINK_DIRS :=
INCLUDE_DIRS :=

WITH_PAPI := $(shell if [ "`papi_avail  2>&1 | grep Yes | wc -l`" -ne "0" ]; then echo 1; else echo 0; fi) 
WITH_MYSQL:= 1

include $(PROJECT_ROOT)/settings.mk

BLD ?= debug
COMPILER ?= g++48
PLUGINS += ccache

ifeq ($(WITH_COVERAGE),1)
PLUGINS += coverage
endif

ifeq ($(WITH_PROFILER),1)
PLUGINS += profiler
endif


ifeq ($(WITH_V8),1)
ifndef V8_BASE_DIRECTORY
$(error V8_BASE_DIRECTORY is not defined)
endif
endif

include $(PROJECT_ROOT)/makefiles/config.$(COMPILER).mk
include $(PLUGINS:%=$(PROJECT_ROOT)/mkplugins/%.mk)

RESULT_DIR := $(PROJECT_ROOT)/build
OBJDIR := $(PROJECT_ROOT)/.build/$(BLD)_$(OSNAME)

CPPFLAGS.debug += -DEXPENSIVE_ASSERTIONS
CPPFLAGS.release += -DEXPENSIVE_TESTS -DPRODUCTION -DNDEBUG

CFLAGS.debug +=
CFLAGS.release +=

LDFLAGS.debug +=
LDFLAGS.release +=

COMMON_FLAGS.debug += -O0
COMMON_FLAGS.release += -O3 -march=native
COMMON_FLAGS += -g -Wall -Wextra -Wno-attributes -Wno-unused-parameter -pthread $(COMMON_FLAGS.$(BLD))

CPPFLAGS += -MMD -pipe $(CPPFLAGS.$(BLD))
CFLAGS += $(COMMON_FLAGS) $(CFLAGS.$(BLD))
CXXFLAGS += -std=c++11 $(COMMON_FLAGS) $(CXXFLAGS.$(BLD))

LINK_DIRS += /usr/local/lib
# This should indeed be done by all the components themselves
INCLUDE_DIRS += $(PROJECT_ROOT)/src/lib
LDFLAGS += $(LDFLAGS.$(BLD))

.PHONY          : all clean test ci_test ci_build ci_valgrind_test
.DEFAULT_GOAL   := all


TESTPARAM = --minimal
test:
ci_test: TESTPARAM = --gtest_output=xml:$<.xml
ci_test: test
ci_valgrind_test: TESTPARAM =
ci_valgrind_test: TESTPREFIX = valgrind --leak-check=full --xml=yes --xml-file=$<.memcheck
ci_valgrind_test: test
include $(PROJECT_ROOT)/makefiles/ci.mk

ci_build: ci_steps

%/.fake:
	@mkdir -p $(@D)
	@touch $@

$(RESULT_DIR)/%.a:
	$(call echo_cmd,AR $(AR) $@) $(AR) crs $@ $(filter %.o,$?)

$(RESULT_DIR)/%:
	$(call echo_cmd,LINK $(CXX) $(BLD) $@) $(CXX) $(CXXFLAGS) -o $@ $(filter %.o,$^) -Wl,-whole-archive $(addprefix -l,$(LIBS)) -Wl,-no-whole-archive $(addprefix -L,$(LINK_DIRS)) $(LDFLAGS)

# Necessary to allow for a second expansion to create dirs
.SECONDEXPANSION:
test: $$(test-tgts)

all: $$(all)
	@echo "$@ done for BLD='$(BLD)'"

clean:
	rm -rf $(OBJDIR) $(all)


$(OBJDIR)%.cpp.o : %.cpp | $$(@D)/.fake
	$(call echo_cmd,CXX $(CXX) $(BLD) $<) $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(addprefix -I,$(INCLUDE_DIRS)) -c -o $@ $<

$(OBJDIR)%.c.o : %.c | $$(@D)/.fake
	$(call echo_cmd,CC $(CC) $(BLD) $<) $(CC) $(CPPFLAGS) $(CFLAGS) $(addprefix -I,$(INCLUDE_DIRS)) -c -o $@ $<

# Ensure that intermediate files (e.g. the foo.o caused by "foo : foo.c")
#  are not auto-deleted --- causing a re-compile every second "make".
.SECONDARY  	:
.SUFFIXES:
%.              :;@echo '$($*)'

ifdef build_debug
$(info TOP is $(TOP))
$(info out of tree build dir $(OBJDIR))
$(info PROJECT_ROOT is $(PROJECT_ROOT))
endif

endif
