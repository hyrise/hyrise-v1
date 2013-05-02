********************
Building the project
********************

A simple::

    $ make

should suffice to build the complete project

Additional targets
==================

make test
    run all tests, stop on error

make test_basic
    run all tests except for tpcc tests, stop on error

make test_ignore_errors
    run all tests, don't stop on error


Adding new library/binary targets
=================================

