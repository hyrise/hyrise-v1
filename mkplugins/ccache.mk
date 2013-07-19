CC := ccache $(CC)
CXX:= ccache $(CXX)

export CCACHE_SLOPPINESS=time_macros
