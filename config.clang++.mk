export CCACHE_CPP2 := yes

BUILD_FLAGS += -fcolor-diagnostics -Qunused-arguments -Wno-covered-switch-default 

# Workaround as per http://llvm.org/bugs/show_bug.cgi?id=12730
BUILD_FLAGS += -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8

CXX := clang++
CC := clang
LD := clang++
