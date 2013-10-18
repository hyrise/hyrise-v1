hyr-units-taskscheduler := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../../rules.mk

include $(PROJECT_ROOT)/src/lib/taskscheduler/Makefile
include $(PROJECT_ROOT)/src/lib/access/Makefile
include $(PROJECT_ROOT)/third_party/gtest/Makefile

hyr-units-taskscheduler.binname := units-taskscheduler
hyr-units-taskscheduler.deps := hyr-taskscheduler hyr-access gtest
$(eval $(call test-binary,hyr-units-taskscheduler))
