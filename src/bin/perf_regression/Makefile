hyr-perf := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../../rules.mk

include $(PROJECT_ROOT)/src/lib/access/Makefile
include $(PROJECT_ROOT)/third_party/Makefile
include $(PROJECT_ROOT)/src/bin/perf_datagen/Makefile

hyr-perf.binname := hyrise-perf
hyr-perf.deps := hyr-access gtest
#hyr-perf.libs := boost_program_options
$(eval $(call binary,hyr-perf))
all: $(hyr-perf.binary)

.PHONY: codespeed
codespeed: $(hyr-perf.binary) $(hyr-perf-datagen.binary)
	echo $?
	mkdir -p $(PROJECT_ROOT)/benchmark_data
	$(hyr-perf-datagen.binary) -w 10 -d $(PROJECT_ROOT)/benchmark_data --hyrise
	HYRISE_DB_PATH=$(PROJECT_ROOT)/benchmark_data $(hyr-perf.binary) --gtest_catch_exceptions=0 --gtest_output=xml:benchmark.xml
