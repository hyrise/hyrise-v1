available := $(shell which g++-4.7 g++-4.8 g++-5)

ifneq ($(findstring g++-5,$(available)),)
include makefiles/config.g++5.mk
else ifneq ($(findstring g++-4.8,$(available)),)
include makefiles/config.g++48.mk
else ifneq ($(findstring g++-4.7,$(available)),)
include makefiles/config.g++47.mk
else
$(error No suitable g++ (either 4.7, 4.8 or 5) found. Explicitly select your compiler in settings.mk COMPILER setting)
endif
