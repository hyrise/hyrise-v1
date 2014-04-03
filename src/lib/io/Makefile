hyr-io := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../../rules.mk

include $(PROJECT_ROOT)/third_party/Makefile
include $(PROJECT_ROOT)/src/lib/helper/Makefile
include $(PROJECT_ROOT)/src/lib/storage/Makefile
include $(PROJECT_ROOT)/src/lib/taskscheduler/Makefile

hyr-io.libname := hyr-io
hyr-io.libs := csv boost_system boost_filesystem
hyr-io.deps := hyr-helper hyr-storage hyr-taskscheduler cereal optional json

ifeq ($(WITH_MYSQL), 1)
hyr-io.libs += mysqlclient
hyr-io.CPPFLAGS += -DWITH_MYSQL
endif

$(eval $(call library,hyr-io))
