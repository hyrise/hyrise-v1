hyr-io := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../../rules.mk

include $(PROJECT_ROOT)/src/lib/helper/Makefile
include $(PROJECT_ROOT)/src/lib/storage/Makefile
include $(PROJECT_ROOT)/src/lib/taskscheduler/Makefile
include $(PROJECT_ROOT)/third_party/Makefile

hyr-io.libname := hyr-io
hyr-io.libs := csv
hyr-io.deps := hyr-helper hyr-storage hyr-taskscheduler cereal optional

ifeq ($(WITH_MYSQL), 1)
hyr-io.libs += mysqlclient
hyr-io.CPPFLAGS += -DWITH_MYSQL
endif

$(eval $(call library,hyr-io))
