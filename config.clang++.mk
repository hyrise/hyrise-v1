export CCACHE_CPP2 := yes

BUILD_FLAGS := -fcolor-diagnostics -Qunused-arguments -Wno-covered-switch-default -Weverything -Wno-c++98-compat

CXX := clang++
CC := clang
LD := clang++
