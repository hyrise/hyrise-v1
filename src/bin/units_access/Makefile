hyr-units-access := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../../rules.mk

include $(PROJECT_ROOT)/src/lib/access/Makefile
include $(PROJECT_ROOT)/src/lib/testing/Makefile
include $(PROJECT_ROOT)/third_party/gtest/Makefile

hyr-units-access.binname := units-access
hyr-units-access.deps := hyr-access hyr-testing gtest
$(eval $(call test-binary,hyr-units-access))
