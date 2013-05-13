export CCACHE_CPP2 := yes

BUILD_FLAGS := -fcolor-diagnostics -Qunused-arguments -Wno-covered-switch-default -Weverything

CXX := clang++
CC := clang
LD := clang++
