hyr-memory := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../../rules.mk

include $(PROJECT_ROOT)/third_party/Makefile

hyr-memory.libname := hyr-memory
hyr-memory.libs :=
hyr-memory.deps :=
$(eval $(call library,hyr-memory))
