import bisect
import csv
import random
import sys

from collections import defaultdict
from datetime import datetime
from hashlib import md5
from optparse import OptionParser

#@profiling
def main():
    usage = """usage: python datagenerator.py -c 10 -r 1000 -f filename -i filename_index"""
    
    parser = OptionParser(usage=usage)
    
    parser.add_option("-f", "--filename", dest="filename", action="store",
                        help="Output CSV file", default="generated_data.csv")
    
    parser.add_option("-i", "--filename_index", dest="filename_index", action="store",
                        help="Index output CSV file", default="generated_data_index.csv")
    
    parser.add_option("-d", "--distinct", dest="distinct", action="store",
                        help="Number of distinct values for the placeholder columns (actual number, not percentage)", default=None)
    
    parser.add_option("-r", "--rows", dest="rows", action="store",
                        help="Amount of rows in the table", default="1000")
    
    parser.add_option("-c", "--columns", dest="columns", action="store",
                        help="Number of columns in the table. This involves the data column.\
                                --columns=1 will create a CSV file with only the data column \
                                being generated.", default="10")
    
    #args = len(sys.argv) > 1 and sys.argv[1:]
    (options, args) = parser.parse_args(sys.argv)
    if len(sys.argv) == 1:
        parser.print_help()
        return
    
    print "rows:",options.rows
    print "columns:",options.columns
    print "filename:",options.filename
    
    print "Generating two CSV files - %s and %s, layout is: <material id>, <amount>, <random column> (as many random columns as specified in --columns, minus two)" % (options.filename,
                                                                                                                                                                       options.filename_index)
    # initialize some settings
    num_columns = int(options.columns)
    distinct_values = int(options.distinct)
    
    # num_columns is always at least 2, since those are the material id number and the amount column
    num_columns = max(2,num_columns)
    
    # seed is a random float value that is appended before the md5 hash generation
    # due to this the script will produce different md5 hashes for the same int
    # values every time.
    seed = random.random()
    
    # random number for the 'amount' field. a amount involved n items of this material
    def random_amount_number():
        return random.randint(1,100)
    
    
    # open the csv file and provide a function that has all the necessary settings already mixed into it
    f = open(options.filename, 'w+')
    writer = csv.writer(f, delimiter='|', quotechar='"', quoting=csv.QUOTE_MINIMAL)

    # write header
    for col in range(int(options.columns)-1):
        f.write("col_" + str(col) + "|")
    f.write("col_" + str(int(options.columns)-1) +"\n")
    for col in range(int(options.columns)-1):
        f.write(" INTEGER |")
    f.write(" INTEGER\n")
    for col in range(int(options.columns)-1):
        f.write(" " + str(col) + "_C |")
    f.write(" " + str(int(options.columns)-1) + "_C\n")
    f.write("===\n")

    distinct_rg = xrange(distinct_values)
    
    def store_row(value):
        rest_row = []
        for x in range(num_columns-2):
            rest_row += random.sample(distinct_rg, 1)
        writer.writerow([value, random_amount_number()] + rest_row)
    
    # the inverse index is a dictionary of position lists
    index = defaultdict(list)
    
    # every row will now be generated and stored in the csv
    p = Percentage(int(options.rows))
    for doc_id in range(int(options.rows)):
        p.tick()
        value = int((random.expovariate(3000) * 60000) ** 2.5)
        #value = md5('%s%i' % (seed, value)).hexdigest()
        
        store_row(value)
        index[value].append(doc_id)

    # 
    # print ""
    # print "Generated Data, now index"
    # 
    # # open the csv file and provide a function that has all the necessary settings already mixed into it
    # f2 = open(options.filename_index, 'w+')
    # writer2 = csv.writer(f2, delimiter='|', quotechar='"', quoting=csv.QUOTE_MINIMAL)
    # def store_row_index(key, document_ids):
    #     writer2.writerow([key, ','.join([str(x) for x in document_ids])])
    # 
    # # Generate the index
    # p = Percentage(int(len(index.keys())))
    # for key, document_ids in sorted(index.items(), key=lambda x:len(x[1]), reverse=True):
    #     p.tick()
    #     store_row_index(key, document_ids)





def profiling(profiling_func):
    def profiling_decorator_func(*args, **kwargs):
        import hotshot
        prof = hotshot.Profile("hotshot_edi_stats")
        ret = prof.runcall(profiling_func,*args)
        prof.close()
        
        from hotshot import stats
        print "now printing stats..."
        s = stats.load("hotshot_edi_stats")
        s.sort_stats("time").print_stats(15)
        s.sort_stats("cum").print_stats(15)
        return ret
    return profiling_decorator_func

class Percentage(object):
    def __init__(self,total,stepsize=0.01):
        self.total = float(total)
        self.last_percent = 0
        self.step = stepsize
        self.count = 0
        self.next_print = (self.total / (100 / self.step))
        self.start = datetime.now()

    def tick(self,num=1):
        self.count += num
        if self.count > self.next_print:
            self.next_print += (self.total / (100 / self.step))
            fac = (self.count/float(self.total))
            td = (datetime.now() - self.start)
            td = td.seconds + td.microseconds/1000000.0
            remaining = td/fac - td
            print >> sys.stderr,"                             %.2f %% (%.2fs remaining)" % (fac*100,remaining),'\r',
            sys.stderr.flush()
            return True
        return False


if __name__ == "__main__":
    main()
    print ""
