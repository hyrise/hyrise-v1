import sys
import os, fnmatch, re

LICENSE = "// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved."
SUFFIXES = ["*.h", "*.hpp", "*.cc", "*.cpp", "*.c"]
FOLDERS = ["src"]


def main(filenames):
    for filename in filenames:
        with open(filename) as original: data = original.read()
        if data.split('\n')[0] != LICENSE:
           print "Adding license to", filename
           with open(filename, 'w') as modified: modified.write(LICENSE + "\n" + data)



def files():
	matches = []
	for f in FOLDERS:
		for root, dirnames, filenames in os.walk(f):
			for p in SUFFIXES:
				matches += [os.path.join(root, x) for x in fnmatch.filter(filenames, p)]			
	return matches

if __name__=="__main__":
    check_files=  files()
    main(check_files)