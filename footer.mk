ifneq ($(VERBOSE_BUILD), 1)
ifeq ($(COLOR_TTY), 1)
echo_prog := $(shell if echo -e | grep -q -- -e; then echo echo; else echo echo -e; fi)
echo_cmd = @$(echo_prog) "$(COLOR)$(subst $(IMH_PROJECT_PATH)/,,$(1))$(NOCOLOR)";
else
echo_cmd = @echo "$(1)";
endif
else # Verbose output
echo_cmd =
endif

.PHONY:: clean

ifdef libname
lib ?= $(BUILD_DIR)lib$(libname).$(LIB_EXTENSION)
endif

ifdef binname
bin ?= $(BUILD_DIR)$(binname)
endif

include_dirs += $(PROJECT_INCLUDE)
linker_dirs += $(LINKER_DIR)
sources ?= $(shell find $(src_dir) -type f -name "*.c" -or -name "*.cpp" -and -not -name ".*" | grep -v .skeleton.cpp$$ )
objects ?= $(subst $(src_dir)/,,$(subst .c,.o,$(subst .cpp,.o,$(sources))))
dependencies ?= $(subst .o,.d,$(objects))

include_flags = $(addprefix -I, $(include_dirs))
linker_dir_flags = $(addprefix -L, $(linker_dirs))

makefiles := $(shell find $(BUILD_DIR) -type f -name Makefile) $(IMH_PROJECT_PATH)/*.mk

VPATH := $(src_dir)

all:: $(lib) $(bin)

$(bin): $(objects)
	$(call echo_cmd,BINARY $@) $(LD) -o $@ $(objects) $(LINKER_FLAGS) $(BINARY_LINKER_FLAGS) $(lib_dependencies) $(linker_dir_flags)

$(lib): $(objects) 
	$(call echo_cmd,LINK $@) $(LD) $(SHARED_LIB) -o $@ $(objects) $(LINKER_FLAGS) $(lib_dependencies) $(linker_dir_flags)

%.o: %.cpp $(makefiles)
	$(call echo_cmd,CXX $<) $(CXX) $(CXX_BUILD_FLAGS) $(include_flags) -c -o $@ $< 

%.o: %.c $(makefiles)
	$(call echo_cmd,CC $<) $(CC) $(CC_BUILD_FLAGS) $(include_flags) -c -o $@ $< 

%.d : %.cpp
	$(call echo_cmd,DEP $<) $(CXX) -MF"$@" -MM $(CXX_BUILD_FLAGS) $(CXX_DEBUG) $(include_flags) "$<"

%.d : %.c
	$(call echo_cmd,DEP $<) $(CXX) -MF"$@" -MM $(CC_BUILD_FLAGS) $(CC_DEBUG) $(include_flags) "$<"

clean::
	-$(call echo_cmd,CLEAN $(bin)$(lib)) $(RM) -rf $(src_dir)/*.d $(src_dir)/*.o $(objects) $(dependencies) $(bin) $(lib)

ifneq "$(MAKECMDGOALS)" "clean"
    -include $(dependencies)
endif
