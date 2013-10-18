available := $(shell which g++-4.7 g++-4.8)
ifeq (available,)
$(error No suitable g++ (either 4.7 or 4.8) found. Explicitly select your compiler in settings.mk COMPILER setting)
endif

ifneq ($(findstring g++-4.8,$(available)),)
include makefiles/config.g++48.mk
else
ifneq ($(findstring g++-4.7,$(available)),)
include makefiles/config.g++47.mk
endif
endif
