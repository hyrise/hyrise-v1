gtest := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../rules.mk

gtest.libname := hyr-gtest
gtest.includes := $(realpath $(gtest)/..)
gtest.libs := pthread log4cxx
$(eval $(call library,gtest))
