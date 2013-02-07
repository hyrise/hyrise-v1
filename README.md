# HYRISE

Traditional databases are separated into ones for current data from the day-to-
day business processes and ones for reporting and analytics. For fast moving
businesses moving data from one silo to another is cumbersome and takes too
much time. As a result the new data arriving in the reporting system is already
old by the time it is loaded. HYRISE proposes a new way to solve this problem:
It analyzes the query input and reorganizes the stored data in different
dimensions.

In detail, HYRISE partitions the layout of the underlying tables in a vertical
and horizontal manner depending on the input to this layout management
component. The workload is specified as a set of queries and weights and is
processed by calculating the layout dependent costs for those queries. Based on
our cost-model we can now calculate the best set of partitions for this input
workload. This optimization allows great speed improvements compared to
traditional storage models.

This database provides the implementation to the above mentioned ideas. 


## Installation

Currently only Linux systems are supported. Mac OS X could work, but you will
need a more recent compiler than GCC 4.2. Best is to update the compiler using
the homebrew-dupes repository.

Other than that standard build environment should suffice.

### Automatic Dependency Installation

#### Vagrant

Using vagrant and VirtualBox you can have a fully working HYRISE development
setup in the matter of minutes without having to worry about anything. This is
currently our recommendation to setup. If you have installed VirtualBox and Vagrant you should be able to run the init script simply by calling

    vagrant up 

from the command line.

#### Ubuntu

If you are on Ubuntu it should be sufficient to run the `install.sh` script from the `tools/autosetup` folder. 
  
## Building

To start the build you will only need to execute a simple command line command
to start. The given make file provides all necessary targets. First check if
all submodules are initialized correctly.

    git submodule init --update

<<<<<<< HEAD
<<<<<<< HEAD
Now copy the file `settings.mk.default` to `settings.mk` and if necessary
modify settings to start the build. The most prominent one is `PRODUCTION` to
disable the debug build and `HYRISE_ALLOCATOR` for using `tcmalloc` or
`jemalloc` instead of the libc allocator.

=======
>>>>>>> Preparing HYRISE for OS release
=======
Now copy the file `settings.mk.default` to `settings.mk` and if necessary
modify settings to start the build. The most prominent one is `PRODUCTION` to
disable the debug build and `HYRISE_ALLOCATOR` for using `tcmalloc` or
`jemalloc` instead of the libc allocator.

>>>>>>> Updating README
A simple build is executed by

    make

To build and execute the test suites using predefined tables issue

    make test

The smaller test-suite can be executed using

    make test_basic

## Documentation

The project provides some documentation about how to develop and use HYRISE.
The documentation can be build using.

    make docs

The documentation contains information about how to use HYRISE and how to
implement own plan operations and table types etc. If you have any questions
feel free to post in

    hyrise-dev@googlegroups.com

To contact fellow hyrise developers.

## How to Contribute

Pull requests and issues reports are always welcome. The easiest way to
participate is to clone HYRISE and submit patches that we can merge including
tests.

If you have any questions feel free to contact the maintainers

  * Martin Grund (@grundprinzip)
  * Jens Krueger (@jnkrueger)
  * Johannes Wust
  * Sebastian Hillig (@bastih)


## Contributers

The following people contributed to HYRISE in various forms listed in
alphabetical order :)

  * Alexander Franke
  * Christian Tinnefeld
  * David Eickhoff
  * David Schwalb
  * Friedhelm Filler
  * Henning Lohse
  * Holger Pirk
  * Jan Oberst
  * Jens Krueger
  * Johannes Wust
  * Jonas Witt
  * Marco Hornung
  * Martin Boissier
  * Martin Faust
  * Martin Grund
  * Martin Linkhorst
  * Marvin Killing
  * Matthias Lleine
  * Maximilian Schneider
  * Robert Strobl
  * Sebastian Blessing
  * Sebastian Hillig
  * Sebastian Klose
  * Tim Berning
  * Tim Zimmermann
  * Uwe Hartmann

## License

HYRISE is licensed as open source after the OpenSource "Licence of the Hasso-Plattner Institute" declared in the LICENSE file of this project. 

Question: Why does HYRISE not use MIT or BSD or XX license. The reason for our
approach comes with the German copyright law. Common BSD and MIT licenses are
not necessarily compatible and thus can potentially lead to problems. To
overcome this problem this project uses an specifically designed open source
license to be compatible with German law. The most prominent difference is the
exclusion of all liabilities which is not possible in Germany.
