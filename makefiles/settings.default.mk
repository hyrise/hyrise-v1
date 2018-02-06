# Build settings for Hyrise are controlled with environment variables
# Default settings are specified in rules.mk
# However, setting in this file have precedence
COMPILER ?= autog++
# options: debug|release
BLD ?= debug

# WITH_COVERAGE := 1
# WITH_PAPI := 0
# WITH_V8 := 0
# WITH_PROFILER := 1

# Per Default HYRISE is compiled without MySQL support, set to 1 to enable
# WITH_MYSQL ?= 1

# options: NONE|BUFFEREDLOGGER
# PERSISTENCY ?= BUFFEREDLOGGER
# REPLICATION ?= 1

# To enable Link Time Optimization
# PLUGINS += lto

# See mkpluings for more Build Plugins
include mysql.mk
