# Compiler settings for clang

export CCACHE_CPP2 := yes

BUILD_FLAGS := -fcolor-diagnostics -Qunused-arguments -Wno-covered-switch-default

CXX := ccache clang++
CC := ccache clang
LD := clang++
