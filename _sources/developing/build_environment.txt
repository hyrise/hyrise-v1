***********************
Setup Build Environment
***********************

Currently only two platforms are supported: Linux and Mac OS X.

Library Dependencies
====================

The following dependencies are hard and are strictly required to perform a complete HYRISE build.

* boost
* ccache
* google performance tools
* libcsv
* libev
* libhwloc (-dev)
* libantlr3c(-dev)
* log4cxx
* thrift
* libmetis.a (5.0) - http://glaros.dtc.umn.edu/gkhome/metis/metis/download

Optional Dependencies
---------------------

* PAPI
* gnuplot

Initial Git Setup
=================

Make sure git is correctly configured and an ssh key is setup to be
used with `Planio <https://epic.plan.io/projects/hyrise>`_::

    # download and install Git
    git config --global user.name "Johannes Wust" 
    git config --global user.email johannes.wust@hpi.uni-potsdam.de

    git clone git@epic.plan.io:epic-hyrise.git

    git submodule init
    git submodule update


Linux (Ubuntu)
==============

For Ubuntu, we provide a set of automatic scripts to install all
necessary dependencies to start development with HYRISE::

    $ cd tools/autosetup
    $ ./install.sh

This should suffice to install all packages and external dependencies.

Mac OS X
========

Prerequisites
-------------

Using Mac OS X ist is recommended to install the dependencies using
`homebrew <https://github.com/mxcl/homebrew>`_. Most of the
dependencies are available and need no further configuration.

Simple Setup
------------

Copy and paste the following execution recipe::

    mkdir install
    cd install
    brew install boost thrift log4cxx google-perftools ccache hwloc libev cmake
    wget http://leaseweb.dl.sourceforge.net/project/libcsv/libcsv/libcsv-3.0.1/libcsv-3.0.1.tar.gz
    tar zxvf libcsv-3.0.1.tar.gz
    cd libcsv-3.0.1
    make 
    sudo make install
    cd ..
    wget http://www.antlr.org/depot/antlr3/main/runtime/C/dist/libantlr3c-3.1.4-SNAPSHOT.tar.gz
    cd libantlr3c-3.1.4-SNAPSHOT
    ./configure
    make
    sudo make install
    cd ..
    wget http://glaros.dtc.umn.edu/gkhome/fetch/sw/metis/metis-5.0.tar.gz
    tar zxvf metis-5.0.tar.gz
    cd metis-5.0
    make config
    sudo make install
    cd ..
    wget ftp://oss.sgi.com/www/projects/libnuma/download/numactl-2.0.7.tar.gz
    tar zvxf numactl-2.0.7.tar.gz
    cd numactl-2.0.7
    sudo make install    


.. _builprep:

Build Preparation
=================

To finish the build preparation copy the configuraiton template
``settings.mk.default`` to ``settings.mk`` and adjust the values in
this file accordingly.

* ``PRODUCTION`` -- determines if the build is an optimized build and
  should be used for productions, applys O3 and strips debug symbols
* ``USE_GOOGLE_PERF_LIB`` -- uses the Google malloc implementation
  instead of the default malloc
* ``PAPI_TRACE`` -- if PAPI should be used for performance measurements
* ``RETAIN_DETAIL`` -- prints addition debug messages to trace memory
  leaks with ``RETAIN_DEBUG`` enabled.
* ``RETAIN_DEBUG`` -- If this flag is enabled, additional checks are
  performed at the end of each unit test to ensure that no memory is
  left after test execution.
* ``WITH_MYSQL`` -- Adds support for a MySQL based loader. More
  information on this functionality can be found in :doc:`/loading/mysql`

.. note::
    When working with Linux it might be necessary to set the additional
    ``OSTYPE`` environment variable (or set it in the ``settings.mk`` file
    to "linux") to make sure HYRISE builds correctly.

    This is *required* when ``uname`` does not return "Linux" on a Linux system.

Build HYRISE
============

Run ``make`` to compile HYRISE. Further instructions can be found in :doc:`building`.
