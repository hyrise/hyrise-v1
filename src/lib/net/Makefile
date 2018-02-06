ifndef hyr-net
hyr-net := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

include $(hyr-net)/../../../rules.mk

include $(PROJECT_ROOT)/third_party/Makefile
include $(PROJECT_ROOT)/src/lib/taskscheduler/Makefile

NANOMSG_BUILD ?= cd $(PROJECT_ROOT)/third_party/nanomsg; mkdir bin; cd bin; cmake .. -DNN_STATIC_LIB=ON -DCMAKE_INSTALL_PREFIX=$(PROJECT_ROOT)/third_party/nanomsg/bin; cmake --build .; ctest .;cmake --build . --target install; cd $(CURRENT_PATH)

$(PROJECT_ROOT)/third_party/nanomsg/bin/lib: $(PROJECT_ROOT)/third_party/nanomsg
	$(NANOMSG_BUILD)

TOOLING += $(PROJECT_ROOT)/third_party/nanomsg/bin/lib

hyr-net.libname := hyr-net
hyr-net.includes += $(PROJECT_ROOT)/third_party/nanomsg/bin/include
hyr-net.LINK_DIRS += $(PROJECT_ROOT)/third_party/nanomsg/bin/lib
hyr-net.deps := json ebb hyr-taskscheduler optional cereal
hyr-net.libs := boost_filesystem boost_system nanomsg
$(eval $(call library,hyr-net))
endif
