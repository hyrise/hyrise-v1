hyr-net := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../../rules.mk

include $(PROJECT_ROOT)/third_party/Makefile
include $(PROJECT_ROOT)/src/lib/taskscheduler/Makefile

hyr-net.libname := hyr-net
hyr-net.deps := json ebb hyr-taskscheduler
hyr-net.libs := boost_filesystem boost_system
$(eval $(call library,hyr-net))
