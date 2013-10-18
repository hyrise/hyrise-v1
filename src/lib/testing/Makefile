hyr-testing := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../rules.mk

include $(PROJECT_ROOT)/third_party/gtest/Makefile
include $(PROJECT_ROOT)/third_party/Makefile

hyr-testing.libname := hyr-testing
hyr-testing.deps := gtest ftprinter
$(eval $(call library,hyr-testing))
