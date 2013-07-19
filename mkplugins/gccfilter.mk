GCCFILTER := $(IMH_PROJECT_PATH)/tools/gccfilter -c -p
CC := $(GCCFILTER) $(CC)
CXX:= $(GCCFILTER) $(CXX)
