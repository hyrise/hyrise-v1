hyr-thrift-server := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../../rules.mk

include $(PROJECT_ROOT)/src/lib/access/Makefile
include $(PROJECT_ROOT)/src/lib/net/Makefile

hyr-thrift-server.binname := hyrise-server-thrift
hyr-thrift-server.deps := hyr-access
hyr-thrift-server.libs := boost_program_options thrift
$(eval $(call full_link_binary,hyr-thrift-server))

all: $(hyr-thrift-server.binary)




