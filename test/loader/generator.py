import string
import random
import sys
import os
import shutil
import os.path
from optparse import OptionParser

def id_generator(size=6, chars=string.ascii_uppercase + string.digits):
    return ''.join(random.choice(chars) for x in range(size))


def create_header(name, cols):
    names = []
    types = " | ".join(["INTEGER"] * cols)
    parts = []

    for x in range(cols):
        names.append("COL"+ str(x))
        parts.append(str(x) + "_C")

    data = ""
    data += " | ".join(names) + "\n"
    data += types + "\n"
    data += " | ".join(parts) + "\n"
    return data


def create_file(name, rows, distinct):
    fid = open(name, "w+")
    distinct_rg = xrange(distinct)

    for x in range(rows):
        fid.write(str(random.sample(distinct_rg, 1)[0]))
        fid.write("\n")

    fid.close()

def main():


    usage = """usage: python generator.py -r 1000 -d 20"""
    
    parser = OptionParser(usage=usage)

    parser.add_option("-n", "--name", dest="name", action="store")

    parser.add_option("-c", "--cols", dest="cols", action="store")

    parser.add_option("-d", "--distinct", dest="distinct", action="store",
                        help="Number of distinct values for the placeholder columns (actual number, not percentage)", default=None)
        
    parser.add_option("-r", "--rows", dest="rows", action="store",
                        help="Amount of rows in the table", default="1000")
    

    (options, args) = parser.parse_args(sys.argv)
    if len(sys.argv) == 1:
        parser.print_help()
        return
    
    # create directory
    shutil.rmtree(options.name, True)
    os.mkdir(options.name)

    fid = open(os.path.join(".", options.name, options.name + ".tbl"), "w+")
    fid.write(create_header(options.name, int(options.cols)))
    fid.close

    for x in range(int(options.cols)):
        create_file(os.path.join(".", options.name, "COL" + str(x) + ".data"), int(options.rows), int(options.distinct))

if __name__ == "__main__":
    main()

