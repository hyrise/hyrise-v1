hyr-units-memory := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../../rules.mk

include $(PROJECT_ROOT)/src/lib/memory/Makefile
include $(PROJECT_ROOT)/third_party/gtest/Makefile

hyr-units-memory.binname := units-memory
hyr-units-memory.deps := gtest
$(eval $(call test-binary,hyr-units-memory))
