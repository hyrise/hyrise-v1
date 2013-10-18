hyr-units-storage := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../../rules.mk

include $(PROJECT_ROOT)/src/lib/storage/Makefile
include $(PROJECT_ROOT)/src/lib/io/Makefile
include $(PROJECT_ROOT)/src/lib/testing/Makefile
include $(PROJECT_ROOT)/third_party/gtest/Makefile

hyr-units-storage.binname := units-storage
hyr-units-storage.deps := hyr-storage hyr-io hyr-testing gtest
$(eval $(call test-binary,hyr-units-storage))

