hyr-taskscheduler := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../../rules.mk

include $(PROJECT_ROOT)/third_party/Makefile

hyr-taskscheduler.libname := hyr-taskscheduler
hyr-taskscheduler.libs := tbb
hyr-taskscheduler.deps := hyr-helper
$(eval $(call library,hyr-taskscheduler))
