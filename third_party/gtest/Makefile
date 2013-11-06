gtest := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../rules.mk

include $(PROJECT_ROOT)/src/lib/helper/Makefile

gtest.libname := hyr-gtest
gtest.includes := $(realpath $(gtest)/..)
gtest.deps := hyr-helper
gtest.libs := pthread log4cxx
$(eval $(call library,gtest))
