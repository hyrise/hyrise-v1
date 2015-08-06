if [ -n "`git status --porcelain src/bin/perf_regression/InsertScan.cpp`" ]
then
	echo InsertScan.cpp has uncommited changes, aborting benchmark
	exit
fi

if [ -n "`git status --porcelain src/lib/io/logging.h`" ]
then
	echo logging.h has uncommited changes, aborting benchmark
	exit
fi

BENCHMARKFILE=${1-test/test10k_12.tbl}
echo "Benchmark file: $BENCHMARKFILE"
if [ ! -f $BENCHMARKFILE ]
then
    echo File does not exists.
    exit
fi

echo Number of records: $((`wc -l $BENCHMARKFILE | cut -d " " -f 1` - 4))
echo "Reporting mean runtime in usec"
echo

if [ -n "`grep 'PRODUCTION := 0' settings.mk`" ]
then
	echo "PRODUCTION seems to be set to 0 in settings.mk. Are you sure you want to run the benchmark?"
fi

PROCESSORS=`grep -c ^processor /proc/cpuinfo`

printf "%-20s" Logger
printf "%25s" `sed -n 's/BENCHMARK_F(InsertScanBase, \(.*\)).*/\1/p' src/bin/perf_regression/InsertScan.cpp | sed 's/insert_//'`
echo

for LOGGER in `cat src/lib/io/logging.h | sed -n "s/.*typedef \(.\+\) Logger;.*/\1/p"`
do
	sed -i 's#test/test10k_12.tbl#'${BENCHMARKFILE}'#g' src/bin/perf_regression/InsertScan.cpp
	sed -ie "s#^  \(typedef .* Logger;\)#  //\1#" src/lib/io/logging.h
	sed -ie "s#^  //\(typedef $LOGGER Logger;\)#  \1#" src/lib/io/logging.h
	make -j $PROCESSORS > /dev/null 2>/dev/null
	[ $? -ne 0 ] && echo make failed && exit
	printf "%-20s" $LOGGER
	HYRISE_DB_PATH=`pwd` LD_LIBRARY_PATH=`pwd`/build build/perf_regression --gtest_filter=*Insert* --gtest_catch_exceptions=0 2>/dev/null | grep RUNTIME_MEAN | cut -d ":" -f 2 | tr -d " " | xargs printf "%25.0f"
	echo
	git checkout src/bin/perf_regression/InsertScan.cpp src/lib/io/logging.h
done

make -j $PROCESSORS > /dev/null 2>/dev/null
[ $? -ne 0 ] && echo make failed && exit