tp := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../rules.mk

ftprinter := $(tp)/ftprinter

ftprinter.cpps := $(filter-out %test_main.cpp,$(wildcard $(ftprinter)/src/*.cpp))
ftprinter.libname := ftprinter
ftprinter.includes := $(ftprinter)/include
$(eval $(call library,ftprinter))

cereal.use_INCLUDE_DIRS := $(tp)/cereal/include

ebb := $(tp)/ebb
ebb.cpps := $(addprefix $(ebb)/,ebb.c ebb_request_parser.c)
#ebb.objs := $(ebb.cpps:%=$(OBJDIR)%.o)
ebb.includes := $(tp)
ebb.libname := ebb
ebb.libs := ev
$(eval $(call library,ebb))

json := $(tp)/jsoncpp
json.libname := json
json.includes := $(tp)/jsoncpp
$(eval $(call library,json))

optional.use_INCLUDE_DIRS := $(tp)/optional
