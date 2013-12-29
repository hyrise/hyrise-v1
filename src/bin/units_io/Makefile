hyr-units-io := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../../rules.mk

include $(PROJECT_ROOT)/src/lib/io/Makefile
include $(PROJECT_ROOT)/src/lib/testing/Makefile
include $(PROJECT_ROOT)/third_party/gtest/Makefile

hyr-units-io.binname := units-io
hyr-units-io.deps := hyr-io hyr-testing gtest
hyr-units-io.libs := boost_filesystem boost_system
$(eval $(call test-binary,hyr-units-io))
