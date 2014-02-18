###############################
Writing Performance Regressions
###############################


Performance regressions are used to keep track of the systems
performance by defining a set of (micro-)benchmarks that execute
specici scenarios relevant to your application. The result of these
benchmarks is then compared over time and gives better understanding
of how architectural changes impact the overall performance.


The Performance Test Data
=========================

None of the performance test data is checked into the code repository.
Therefore, the test data needs to be generated before executing
performance regressions. Basically, generated data that is derived from the 
standard TPC-C reference is used. The HYRISE StorageManager loads this test 
data by reading it's environment variable ``$HYRISE_DB_PATH``. This variable has 
to be set to the path that the data generator used as ouput folder - otherwise
the performance regression suite will abort.


Generating Data
---------------

For generating data a patched version of the open source DBT-2 TPC-C datagen is used.
The compiled binary is located at ``./build/perf_datagen``. 

The usage is as follows::

    ./build/perf_datagen -w # [-c #] [-i #] [-o #] [-s #] [-n #] -d <str> --hyrise
       -> -w => warehouse cardinality
       -> -c => customer cardinality (default: 3000)
       -> -i => item cardinality (default: 100000)
       -> -o => order cardinality (default: 3000)
       -> -n => new_order cardinality (default: 900)
       -> -d => output path for generated tables (default: .)

Generating a table structure with one warehouse entry would look like the following:
``./build/perf_datagen -w 1 -d /tmp/ --hyrise``
The cardinality for the other tables is set by default as described above.


HYRISE Performance Regressions
==============================

HYRISE uses a patched version of Google Testing Framework to allow
benchmarking. Basically all features of standard fixtures are allowed,
however the method chain is slightly different for a benchmark. In the
normal Google Testing Framework the ``TEST_F`` macro defines a class
where the body of the method ``TestBody()`` is implemented. For a
benchmark, we use a pre-defined ``TestBody`` implementation that
allows to execute a test multiple times with a given number of warm up
iterations. The Code that is associated to the ``BENCHMARK`` macro is
executed and measured.


Writing a Simple Regression
---------------------------

One of the easiest possibe performance regressions is defined below::

    BENCHMARK(Simple)
    {
        int i = 10;
        srand(99);
        usleep(500000 + rand() % 200000);
    }

and the output of the example run with a simple printer looks like the
following::

    [ RUN      ]  ProjectionBase.Simple 
    [      MSG ] MIN : 20
    [      MSG ] MAX : 35
    [      MSG ] AVG : 26.6
    [      MSG ] STDDEV : 4.32563
    [       OK ]

Here the output lists all kinds of statistical values that help to
identify the robustness and timely errors of an implementation.


Writing a Regression Fixture
----------------------------

Regression fixtures follow the same principle as testing fixtures and
can be seen as such. The only requirement is that a regression fixture
must derive form ``::testing::Benchmark``. A simple example is defined
below::


    class MyFixture : public ::testing::Benchmark 
    {

        BenchmarkSetUp()
        {
        }

        BenchmarkTearDown()
        {
        }


        MyFixture()
        {   
            SetNumIterations(10);
            SetWarmUp(2);
        }
    }
    
    
    BENCHMARK_F(MyFixture, simple2)
    {
        int i = 10;
        srand(99);
        usleep(500000 + rand() % 200000);
    }

It is important to mention that all member variables of ``MyFixture``
should be declared at least as ``protected`` so that they are
accessible by the subclass created by the ``BENCHMARK_`` macro.

It is possible to define ``BenchmarkSetUp`` and ``BenchmarkTearDown``
methods that will be called before each iteration. The global
``SetUp`` and ``TearDown`` method from the parent test object are
accessible as well but will be executed only _once_ in each regresion
execution.


Output Handler
---------------

The output of the regression tests is controlled by so called
``TestEventListener`` classes. Each of theses interfaces provides
hooks to implement different test events. Since the Benchmark is a
unit test, we need to implement these listeners as well. The current
class ``BenchmarkPrinter`` is only able to print some of the
statistical values, but will not output a JMeter compatible XML
format.

Since GTF is not as configurable as we might want, we have to write an
own event listener that will output the required XML. The easiest way
to do this is to copy most of the functionality of the
``::testing::XmlUnitTestResultPrinter`` defined in
``third_party/gtest/gtest-all.cpp``. 


Executing Performance Regressions
---------------------------------

Once the test data has been generated successfully you need to set the hyrise
data source environment variable accordingly. According to the example described
above this is to be done the following:

``export HYRISE_DB_PATH=/tmp``

All performance regressions that are implemented in src/bin/perf_regressions 
can be executed by simply running their executable:

``./build/perf_regression``

The ouput of the performance regressions execution is as described above.


Publishing Results to Codespeed
-------------------------------

In order to keep track of the results of the performance benchmarks,
we set up a Codespeed Center that allows us to plot and analyse 
benchmark results on a revision basis. The Jenkins build script used
on our CI-Server is taking care of publishing the results to the speed
center each time a new build is triggered. Nevertheless, it is possible 
to publish results manually. New results can be committed to codespeed
using the standard JSON output format. Therefore, the performance regression binary
executing all benchmarks is equipped with a parameter ``--toCodespeed`` to produce
JSON output. To publish this results to the Codespeed web interface we use a python
script located at ``./third_party/codespeed/publish_results.py`` executing an HTTP POST.

A possible usage looks like the following::

  ./build/perf_regression --toCodespeed | tee perf_regression_out.json\n 
   && 
   python third_party/codespeed/publish_results.py perf_regression_out.json
