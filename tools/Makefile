tools := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../rules.mk
include $(PROJECT_ROOT)/src/bin/hyrise/Makefile

.PHONY : python_test

python_test: $(hyr-server.binary)
	python $(tools)/test_server.py $<
ifeq ($(PERSISTENCY),NVRAM)
	python $(tools)/test_recovery.py $<
	python $(tools)/test_transactional_recovery/test.py $<
else
	@echo Not testing persistency - enable persistency testing in tools/Makefile.
endif

test-tgts += python_test
