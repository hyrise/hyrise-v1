hyr-helper := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../../rules.mk

hyr-helper.libname := hyr-helper

hyr-helper.libs := hwloc rt

ifeq ($(WITH_PAPI),"1")
hyr-helper.libs += papi
hyr-helper.CPPFLAGS := -DUSE_PAPI_TRACE
endif
$(eval $(call library,hyr-helper))
