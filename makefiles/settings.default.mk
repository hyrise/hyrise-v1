COMPILER ?= autog++
# options: debug|release
BLD ?= debug

# WITH_COVERAGE := 1
# WITH_PAPI := 0
# WITH_V8 := 0
# WITH_PROFILER := 1

# Per Default HYRISE is compiled with MySQL support, set to 0 to disable
# WITH_MYSQL := 0

PERSISTENCY := NONE
WITH_GROUP_COMMIT := 0

# To enable Link Time Optimization
# PLUGINS += lto

# See mkpluings for more Build Plugins

include mysql.mk
