hyr-units := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../../rules.mk

include $(PROJECT_ROOT)/src/lib/helper/Makefile
include $(PROJECT_ROOT)/third_party/gtest/Makefile

hyr-units.binname := units
hyr-units.deps := hyr-helper gtest
$(eval $(call test-binary,hyr-units))
