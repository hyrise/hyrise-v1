##############
Documentation
##############

What is HYRISE
--------------

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


Current status
--------------

This system is a research prototype and by no means meant to be used in
production use and we cannot guarantee that it will work in such environments.
The goal of an early version of HYRISe was to show that for in-memory databases
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


Quick Start
-----------

Currently only Linux systems are supported. Mac OS X could work, but you will
need a more recent compiler than GCC 4.2. Best is to update the compiler using
the homebrew-dupes repository.

Other than that standard build environment should suffice.


Automatic Dependency Installation
---------------------------------


Vagrant
********

Using vagrant and VirtualBox you can have a fully working HYRISE development
setup in the matter of minutes without having to worry about anything. This is
currently our recommendation to setup. If you have installed VirtualBox and
Vagrant you should be able to run the init script simply by calling

.. code-block:: c
  :linenos:

  vagrant up

from the command line.


Ubuntu
******

If you are on Ubuntu it should be sufficient to run the ``install.sh`` script from the ``tools/autosetup`` folder. 
  

Building
--------

To start the build you will only need to execute a simple command line command
to start. The given make file provides all necessary targets. First check if
all submodules are initialized correctly.

.. code-block:: c
  :linenos:

  git submodule update --init

Now copy the file ``settings.mk.default`` to ``settings.mk`` and if necessary
modify settings to start the build. The most prominent one is ``PRODUCTION`` to
disable the debug build and `HYRISE_ALLOCATOR` for using ``tcmalloc`` or
``jemalloc`` instead of the libc allocator.

Now copy the file `settings.mk.default` to ``settings.mk`` and if necessary
modify settings to start the build. The most prominent one is `PRODUCTION` to
disable the debug build and ``HYRISE_ALLOCATOR`` for using ``tcmalloc`` or
``jemalloc`` instead of the libc allocator.

A simple build is executed by

.. code-block:: c
  :linenos:

  make

To build and execute the test suites using predefined tables issue

.. code-block:: c
  :linenos:

  make test

The smaller test-suite can be executed using

.. code-block:: c
  :linenos:

  make test_basic


Documentation
-------------

The project provides some documentation about how to develop and use HYRISE.
The documentation can be build using.

.. code-block:: c
  :linenos:

  make docs

The documentation contains information about how to use HYRISE and how to
implement own plan operations and table types etc. If you have any questions
feel free to post in

  hyrise-dev@googlegroups.com

To contact fellow hyrise developers.


How to Contribute
-----------------

Pull requests and issues reports are always welcome. The easiest way to
participate is to clone HYRISE and submit patches that we can merge including
tests.

If you have any questions feel free to contact the maintainers

  * Martin Grund (`@grundprinzip <https://github.com/grundprinzip>`_)
  * Jens Krueger (`@jnkrueger <https://github.com/jnkrueger>`_)
  * Johannes Wust (`@jwust <https://github.com/jwust>`_)
  * David Schwalb (`@schwald <https://github.com/schwald>`_)
  * Sebastian Hillig (`@bastih <https://github.com/bastih>`_)


Contributers
------------

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
  * Kai Hoewelmeyer
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
