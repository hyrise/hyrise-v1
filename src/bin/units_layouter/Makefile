hyr-units-layouter := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../../rules.mk

include $(PROJECT_ROOT)/src/lib/layouter/Makefile
include $(PROJECT_ROOT)/src/lib/io/Makefile
include $(PROJECT_ROOT)/src/lib/testing/Makefile
include $(PROJECT_ROOT)/third_party/gtest/Makefile

hyr-units-layouter.binname := units-layouter
hyr-units-layouter.deps := hyr-layouter hyr-io hyr-testing gtest
$(eval $(call test-binary,hyr-units-layouter))
