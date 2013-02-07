ifeq ($(DEBUG), 1)
CXX_FLAGS := -g 
else
CXX_FLAGS := -O3 -g
endif

ifeq ($(PAPI),1)
CXX_FLAGS := $(CXX_FLAGS) -lpapi -D USE_PAPI
endif

ifeq ($(NOSSE),1)
CXX_FLAGS := $(CXX_FLAGS)
else
CXX_FLAGS := $(CXX_FLAGS) -msse4.1 -DUSE_SSE
endif

ifeq ($(NONUMA), 1)
CXX_FLAGS := $(CXX_FLAGS)
else
CXX_FLAGS := $(CXX_FLAGS) -lnuma -DUSE_NUMA
endif




all: hycal


hycal: hycal.cpp hycal.h memtrait.h trace.h types.h
	g++ $(CXX_FLAGS) -o hycal hycal.cpp  -lpthread


clean:
	$(RM) hycal

