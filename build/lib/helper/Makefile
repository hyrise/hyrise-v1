src_dir := $(IMH_PROJECT_PATH)/src/lib/helper
libname := helper

ifneq (,$(findstring linux, $(OSTYPE)))
	lib_dependencies := -lrt -lhwloc
else
	lib_dependencies := -lhwloc 
endif

include $(IMH_PROJECT_PATH)/footer.mk
