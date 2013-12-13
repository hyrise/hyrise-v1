hyr-server := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../../rules.mk

include $(PROJECT_ROOT)/src/lib/access/Makefile
include $(PROJECT_ROOT)/src/lib/net/Makefile
-include $(PROJECT_ROOT)/extensions/Makefile # may fail

hyr-server.binname := hyrise-server
hyr-server.deps := hyr-access extensions
hyr-server.libs := boost_program_options
$(eval $(call full_link_binary,hyr-server))

all: $(hyr-server.binary)


# for backward compatibility
ifndef OLD_SERVER
OLD_SERVER := defined
$(RESULT_DIR)/hyrise_server: $(hyr-server.binary)
	cp $< $@
all += $(RESULT_DIR)/hyrise_server
endif
