$(PROJECT_ROOT)/tools/clang34:
	wget -O $(PROJECT_ROOT)/tools/clang34.tar.xz http://llvm.org/releases/3.4/clang+llvm-3.4-x86_64-linux-gnu-ubuntu-13.10.tar.xz
	cd $(PROJECT_ROOT)/tools/; tar -xvf clang34.tar.xz;mv clang+llvm-3.4-x86_64-linux-gnu-ubuntu-13.10 clang34
	rm $(PROJECT_ROOT)/tools/clang34.tar.xz
