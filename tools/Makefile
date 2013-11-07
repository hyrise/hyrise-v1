tools := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../rules.mk
include $(PROJECT_ROOT)/src/bin/hyrise/Makefile

.PHONY : python_test

python_test: $(hyr-server.binary)
	python $(tools)/test_server.py $<

test-tgts += python_test
