hyr-standalone := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../../rules.mk

include $(PROJECT_ROOT)/src/lib/access/Makefile
include $(PROJECT_ROOT)/src/lib/net/Makefile

hyr-standalone.binname := hyrise-server-standalone
hyr-standalone.deps := hyr-access
hyr-standalone.libs := boost_program_options
$(eval $(call full_link_binary,hyr-standalone))

all: $(hyr-standalone.binary)


# for backward compatibility
ifndef OLD_SERVER
OLD_SERVER := defined
$(RESULT_DIR)/hyrise_server: $(hyr-standalone.binary)
	cp $< $@
all += $(RESULT_DIR)/hyrise_server
endif
