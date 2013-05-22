include config.mk

lib_dir = $(build_dir)/lib
bin_dir = $(build_dir)/bin

phase1 := $(build_dir)/phase1

# library dependencies
lib_storage := $(lib_dir)/storage
lib_access  := $(lib_dir)/access
lib_helper  := $(lib_dir)/helper
lib_io      := $(lib_dir)/io
lib_testing := $(lib_dir)/testing
lib_net     := $(lib_dir)/net
lib_layouter:= $(lib_dir)/layouter
lib_memory  := $(lib_dir)/memory
lib_taskscheduler	:= $(lib_dir)/taskscheduler

# third party dependencies
json		:= $(build_dir)/jsoncpp
ext_gtest	:= $(build_dir)/gtest
lib_ebb		:= $(lib_dir)/ebb
lib_ftprinter	:= $(lib_dir)/ftprinter
lib_backward	:= $(bin_dir)/backward

# a list of all libraries
libraries := $(json) $(lib_helper) $(lib_storage) $(lib_access) $(lib_io) $(lib_testing) $(lib_net) $(lib_layouter) $(lib_ebb) $(lib_memory) $(ext_gtest) $(lib_taskscheduler) $(lib_ftprinter) $(lib_backward)

# binary dependencies
# - test suites
unit_tests_helper	:= $(bin_dir)/units_helper
unit_tests_io 		:= $(bin_dir)/units_io
unit_tests_storage 	:= $(bin_dir)/units_storage
unit_tests_access 	:= $(bin_dir)/units_access
unit_tests_taskscheduler	:=	$(bin_dir)/units_taskscheduler
unit_tests_layouter 	:= $(bin_dir)/units_layouter
unit_tests_memory   	:= $(bin_dir)/units_memory
unit_tests_net	        := $(bin_dir)/units_net
perf_regression		:= $(bin_dir)/perf_regression
perf_datagen            := $(bin_dir)/perf_datagen
test_relation_eq	:= $(bin_dir)/test_relation_eq

basic_test_suites := $(unit_tests_helper) $(unit_tests_io) $(unit_tests_storage) $(unit_tests_layouter) $(unit_tests_access) $(unit_tests_taskscheduler) $(unit_tests_memory) $(unit_tests_net) 
aux_test_suites := $(test_relation_eq)

regression_suite := $(perf_regression)
regression_datagen := $(perf_datagen)

all_test_suites := $(complex_test_suites) $(basic_test_suites) $(aux_test_suites)

all_test_binaries := $(subst bin/,,$(all_test_suites))
basic_test_binaries := $(subst bin/,,$(basic_test_suites))
regression_binaries := $(subst bin/,,$(regression_suite))
datagen_binaries    := $(subst bin/,,$(regression_datagen))

# - other binaries
server_hyrise 		:= $(bin_dir)/hyrise
bin_dummy := $(bin_dir)/dummy

binaries :=  $(server_hyrise) $(bin_dummy)
# list all build targets

tgts :=  $(libraries) $(binaries) $(all_test_suites) $(regression_suite) $(regression_datagen) $(phase1)

.PHONY: all $(tgts) tags test test_basic hudson_build hudson_test $(all_test_binaries) doxygen docs

all: $(tgts)

$(tgts):
	@echo 'DIR $@'
	@$(MAKE) $(MAKEFLAGS) --directory=$@

# dependencies betweeen binaries and libraries

$(libraries): $(phase1)
$(lib_ebb):
$(lib_helper):
$(lib_memory):
$(lib_taskscheduler): $(lib_helper)
$(lib_storage): $(lib_helper) $(lib_memory) $(lib_ftprinter) $(ext_gtest) $(lib_ftprinter)
$(lib_io): $(lib_storage) $(lib_helper)
$(lib_access): $(lib_storage) $(lib_helper) $(lib_io) $(lib_layouter) $(json) $(lib_taskscheduler) $(lib_net)
$(lib_testing): $(ext_gtest) $(lib_storage) $(lib_taskscheduler) $(lib_access)
$(lib_net): $(lib_helper) $(json) $(lib_taskscheduler) $(lib_ebb)

$(server_hyrise): $(libraries)
$(unit_tests_helper): $(libraries)
$(unit_tests_io): $(libraries)
$(unit_tests_storage): $(libraries)
$(unit_tests_net): $(libraries)
$(unit_tests_access): $(libraries)
$(unit_tests_taskscheduler): $(libraries)
$(unit_tests_layouter): $(libraries)
$(unit_tests_memory): $(libraries)
$(all_test_suites): $(libraries)
$(perf_regression): $(libraries)
$(perf_datagen): $(libraries)
$(test_relation_eq): $(libraries)

$(binaries): $(libraries)
$(all_test_suites): $(libraries)

# --- Hack: To make the execution of test tasks sensible to errors when
# running taks `test_basic` or `test`, we set an env variable so the following
# rule can be initialized correctly
ERROR_MODE =
ifeq "$(MAKECMDGOALS)" "test_basic"
	ERROR_MODE := stop_on_error
endif

ifeq "$(MAKECMDGOALS)" "test"
	ERROR_MODE := stop_on_error
endif
# end of hack

$(all_test_binaries): $(all_test_suites)
ifeq ($(ERROR_MODE), stop_on_error)
	$@ $(unit_test_params)
else
	-$@ $(unit_test_params)
# Just make sure we don't have linking errors
	$@ --gtest_list_tests > /dev/null
endif

python_test:
	python tools/test_server.py

basic_test_targets := $(basic_test_binaries)
all_test_targets := $(all_test_binaries) python_test

# Test invocation rules
test: unit_test_params = --minimal
test: $(basic_test_targets)
test_verbose: $(basic_test_targets)
test_all: $(all_test_targets)

include makefiles/*.mk

clean_dependencies:
	@find . -type f -name '*.d' -delete

# Clean up: for every target tgt in $(tgts), we create a rule $(tgt)_clean
clean_targets :=
define CLEAN_TGT
clean_targets += $(1)_clean
$(1)_clean:
	@$(MAKE) clean --directory=$(1)  
endef    
$(foreach tgt,$(tgts),$(eval $(call CLEAN_TGT,$(tgt))))

clean: $(clean_targets) clean_dependencies

tags:
	find `pwd`/src -type f | egrep 'c$$|cc$$|cpp$$|h$$|hpp$$' | egrep -v '\.#.*'| etags -

benchmark_data:
	mkdir -p benchmark_data
	$(build_dir)/perf_datagen -w 10 -d benchmark_data --hyrise

codespeed: $(all) benchmark_data
	@echo ' '
	@echo 'Executing performance regression...'
	@echo ' '
	HYRISE_DB_PATH=$(IMH_PROJECT_PATH)/benchmark_data $(build_dir)/perf_regression --gtest_catch_exceptions=0 --gtest_output=xml:benchmark.xml

duplicates:
	-java -jar third_party/simian/simian-2.3.33.jar -excludes="**/units_*/*" -formatter=xml:simian.xml "src/**.cpp" "src/**.h" "src/**.hpp" 

