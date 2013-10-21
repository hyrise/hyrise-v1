COMPILER ?= autog++
# options: debug|release
BLD ?= debug

# WITH_COVERAGE := 1
# WITH_PAPI :=
# WITH_V8 := 0
# WITH_PROFILER := 1

# Per Default HYRISE is compiled with MySQL support, set to 0 to disable
# WITH_MYSQL := 1

include mysql.mk
