src_dir := $(IMH_PROJECT_PATH)/src/lib/taskscheduler
libname := taskscheduler


ifneq (,$(findstring linux, $(OSTYPE)))
libthread := -lpthread_nonshared
else
libthread := -lpthread
endif

lib_dependencies := $(libthread)  -lhwloc -lhelper -ltbb

include $(IMH_PROJECT_PATH)/footer.mk
