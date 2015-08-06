ifndef hyr-storage
hyr-storage := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

include $(hyr-storage)/../../../rules.mk

include $(PROJECT_ROOT)/src/lib/helper/Makefile
include $(PROJECT_ROOT)/third_party/Makefile

hyr-storage.libname := hyr-storage
hyr-storage.libs := hwloc rt
hyr-storage.deps := hyr-helper ftprinter cereal optional json
$(eval $(call library,hyr-storage))
endif
