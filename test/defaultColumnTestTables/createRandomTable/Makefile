#CXX:=/usr/lib/gcc-snapshot/bin/g++

createTable: create.cpp
	$(CXX) -o create create.cpp

debug: create.cpp
	$(CXX) -o create create.cpp -g

release: create.cpp
	$(CXX) -o create create.cpp -O2

clean:
	rm *.o create
