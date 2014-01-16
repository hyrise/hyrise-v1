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

## Current status

This system is a research prototype and by no means meant to be used in
production use and we cannot guarantee that it will work in such environments.
The goal of an early version of HYRISE was to show that for in-memory databases
the flexibility to choose from different physical layouts is of great
importance. In addition, this system should provide the necessary framework for
teaching advanced database techniques to students in an undergraduate and
graduate level.

Our goal is to improve the performance and the set of features to support more
and more capabilities that you would expect from a traditional relational
database. But no promises, when this will happen.

The following features are currently implemented (not always optimal and convenient):

  * Database loading from CSV, HYRISE text files, binary dumps 
  * Most important database operators and indexes including re-compression and dump to disk
  * Automatic layout calculation and layout switch as front-end accessible plan operations

All plan operations are currently hand-coded in JSON an then queried against
the database. SQL compilation is currently not available but somewhere on the
road-map.

## Continuous Integration

Master status: [![Build Status](https://travis-ci.org/hyrise/hyrise.png?branch=master)](https://travis-ci.org/hyrise/hyrise)

## Installation

Supported (actively tested through CI and in development) systems:

* Ubuntu (>13.04)

Likely supported:

* MacOSX 10.7

HYRISE expects a C++11-capable compiler. We recommend g++ >=4.7, testing is done against g++ 4.7, g++ 4.8 and clang 3.2.

### Development Environment Setup

#### Vagrant

Using [VirtualBox](http://virtualbox.org) and [Vagrant](http://vagrantup.com), you can automatically setup a HYRISE development environment. (This is the recommended way of getting started with HYRISE development).

Check out [this repository](https://github.com/hyrise/hyrise_puppet/) to easily setup a development VM.

#### Ubuntu

It should be sufficient to run the `install_$version.sh` script from the `tools/` folder, where version corresponds to a Ubuntu version.
  
## Building

First check if all submodules are initialized correctly:

    git submodule update --init

To start the build you will only need to execute a simple command line command
to start. The given make file provides all necessary targets. 

A simple build is executed by

    make

To build and execute the test suites using predefined tables issue

    make test

### Build settings

The build can be configured through a build settings file `settings.mk`.

Copy the file `settings.mk.default` to `settings.mk` and if necessary
modify settings. The most prominent one is `PRODUCTION` to
disable the debug build and `HYRISE_ALLOCATOR` for using `tcmalloc` or
`jemalloc` instead of the libc allocator.

To enable an optimized production build with g++-4.8, a `settings.mk` file 
may look as follows:

    COMPILER := g++48
    PRODUCTION := 1

When the settings file has been changed, running `make` should result in a
complete rebuild of HYRISE.

More options are discussed in the documentation.

## Documentation

The project provides some documentation about how to develop and use HYRISE.
The documentation can be build using:

    make docs

The documentation contains information about how to use HYRISE and how to
implement own plan operations and table types etc. 

## Mailing list

If you have any questions feel free to post on the developer mailing list:

    hyrise-dev@googlegroups.com

## How to Contribute

Pull requests and issues reports are always welcome. The easiest way to
participate is to clone HYRISE and submit patches that we can merge including
tests.

If you have any questions feel free to contact the maintainers

  * Martin Grund ([@grundprinzip](https://github.com/grundprinzip))
  * Jens Krueger ([@jnkrueger](https://github.com/jnkrueger))
  * Johannes Wust ([@jwust](https://github.com/jwust))
  * David Schwalb ([@schwald](https://github.com/schwald))
  * Sebastian Hillig ([@bastih]((https://github.com/bastih))

## Contributers

The following people contributed to HYRISE in various forms listed in
alphabetical order :)

  * Alexander Franke
  * Christian Tinnefeld
  * Clemens Frahnow
  * David Eickhoff
  * David Schwalb
  * Friedhelm Filler
  * Georg Hoefer
  * Henning Lohse
  * Holger Pirk
  * Jan Oberst
  * Jens Krueger
  * Johannes Wust
  * Jonas Witt
  * Kai Höwelmeyer
  * Karsten Tausche
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

## FAQ 

### Why does HYRISE not use MIT or BSD or XX license?

The reasoning stems from German copyright law. Common BSD and MIT licenses are not necessarily compatible with liability issues in German copyright legislation. Thus, HYRISE uses an specifically designed open source license deemed compatible with German law. The most prominent difference between our license and commonly used BSD or MIT is the inclusion of certain liabilities as dictated by German copyright law.
