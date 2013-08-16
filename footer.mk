ifneq ($(VERBOSE_BUILD),1)
echo_cmd = @echo "$(subst $(IMH_PROJECT_PATH)/,,$(1))";
else # Verbose output
echo_cmd :=
endif

ifdef libname
lib ?= $(BUILD_DIR)lib$(libname).$(LIB_EXTENSION)
endif

ifdef binname
bin ?= $(BUILD_DIR)$(binname)
endif

include_dirs += $(PROJECT_INCLUDE)
linker_dirs += $(LINKER_DIR)
sources ?= $(sort $(shell find $(src_dir) -type f -name "*.c" -or -name "*.cpp" -and -not -name ".*" ))
objects ?= $(addsuffix .o,$(subst $(src_dir)/,,$(sources)))
dependencies ?= $(subst .o,.d,$(objects))

include_flags = $(addprefix -I, $(include_dirs))
linker_dir_flags = $(addprefix -L, $(linker_dirs))

makefiles := $(shell find $(BUILD_DIR) -type f -name Makefile) $(IMH_PROJECT_PATH)/*.mk

precompiled_header_source ?= $(IMH_PROJECT_PATH)/src/lib/stdlib.hpp
precompiled_header ?= $(precompiled_header_source).gch

VPATH := $(src_dir)

.PHONY: all clean

all:: $(precompiled_header) $(lib) $(bin)
	@:

$(bin): $(objects)
	$(call echo_cmd,BINARY $@) $(LD) -o $@ $(objects) $(LINKER_FLAGS) $(BINARY_LINKER_FLAGS) $(lib_dependencies) $(linker_dir_flags)

$(lib): $(objects)
	$(call echo_cmd,LINK $@) $(LD) $(SHARED_LIB) -o $@ $(objects) $(LINKER_FLAGS) $(lib_dependencies) $(linker_dir_flags)

%.cpp.o: %.cpp $(makefiles) $(precompiled_header)
	@mkdir -p $(@D)
	$(call echo_cmd,CXX $<) $(CXX) -MMD -MP $(CXX_BUILD_FLAGS) $(include_flags) -Winvalid-pch -include $(precompiled_header_source) -c -o $@ $<

%.c.o: %.c $(makefiles)
	@mkdir -p $(@D)
	$(call echo_cmd,CC $<) $(CC) -MMD -MP $(CC_BUILD_FLAGS) $(include_flags) -c -o $@ $<

clean::
	-$(call echo_cmd,CLEAN $(bin)$(lib)) $(RM) -rf $(objects) $(dependencies) $(bin) $(lib)

$(precompiled_header): $(precompiled_header_source) $(makefiles)
	$(call echo_cmd,PRECOMPILING $@ $<) $(CXX) $(CXX_BUILD_FLAGS) $(include_flags) $(precompiled_header_source)

ifneq "$(MAKECMDGOALS)" "clean"
    -include $(dependencies)
endif
