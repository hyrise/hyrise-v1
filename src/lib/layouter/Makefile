hyr-layouter := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../../rules.mk

include $(PROJECT_ROOT)/src/lib/storage/Makefile

hyr-layouter.libname := hyr-layouter
hyr-layouter.deps := hyr-storage
hyr-layouter.libs := metis

$(eval $(call library,hyr-layouter))
$(hyr-layouter.objs) : INCLUDE_DIRS += $(PROJECT_ROOT)/third_party
