hyr-units-access := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../../rules.mk

include $(PROJECT_ROOT)/src/lib/access/Makefile
include $(PROJECT_ROOT)/src/lib/testing/Makefile
include $(PROJECT_ROOT)/third_party/gtest/Makefile
-include $(PROJECT_ROOT)/extensions/Makefile # may fail

hyr-units-access.binname := units-access
hyr-units-access.deps := hyr-access hyr-testing gtest extensions
$(eval $(call test-binary,hyr-units-access))
