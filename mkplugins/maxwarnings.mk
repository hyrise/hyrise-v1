ifneq (,$(findstring clang++,$(CXX)))
	BUILD_FLAGS += -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded 
else 
	BUILD_FLAGS += -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wstrict-overflow=5 #-Wswitch-default -Wno-unused -Wsign-conversion -Wsign-promo #-Wshadow crashes on g++-4.7
endif
