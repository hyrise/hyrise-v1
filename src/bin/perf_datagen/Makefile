hyr-perf-datagen := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

include $(hyr-perf-datagen)/../../../rules.mk

hyr-perf-datagen.binname := hyrise-perf-datagen
$(eval $(call full_link_binary,hyr-perf-datagen))
