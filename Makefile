include rules.mk

include $(PROJECT_ROOT)/src/bin/units/Makefile
include $(PROJECT_ROOT)/src/bin/units_storage/Makefile
include $(PROJECT_ROOT)/src/bin/units_io/Makefile
include $(PROJECT_ROOT)/src/bin/units_net/Makefile
include $(PROJECT_ROOT)/src/bin/units_layouter/Makefile
include $(PROJECT_ROOT)/src/bin/units_taskscheduler/Makefile
include $(PROJECT_ROOT)/src/bin/units_access/Makefile
include $(PROJECT_ROOT)/src/bin/hyrise/Makefile
include $(PROJECT_ROOT)/src/bin/perf_regression/Makefile
include $(PROJECT_ROOT)/tools/Makefile

include $(PROJECT_ROOT)/makefiles/docs.mk

