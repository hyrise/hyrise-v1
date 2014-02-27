#######
Loading
#######


EmptyLoader
===========

.. note::

   This Loader is defined in `src/lib/io/EmptyLoader.h` and provides
   EmptyInput and EmptyHeader.


The classes defined in `EmptyLoader.h` are used when no alternative
was specified. They create empty headers or empty table bodies.


CSVLoader
=========

.. note::
   Defined in `src/lin/io/CSVLoader.h` and provides *CSVInput* and
   *CSVHeader*.

This loader can be used to load arbitrary CSV-like files. The default
format of HYRISE uses the delimiter `|`.


Usage
-----

CSVLoader can be used as follows

.. literalinclude:: ../../../src/bin/units_io/csv_tests.cpp
    :language: c++
    :linenos: 


Parameters
==========

`CSVInput::params()` offers the following parameters:

IgnoreHeader : bool
    Ignore four initial lines of header
Unsafe : bool
    Ignore when input dataset is wider than target table, thus
    discarding extra data.
CSVParams : csv::params
    See discussion of csv::params :ref:`csvparams`


`CSVHeader::params()` offers the following parameters:

CSVParams : csv::params
    See discussion of csv::params :ref:`csvparams`

.. _csvparams:


Generic CSV parameters
======================

`csv::params()` offers the following parameters:

Delimiter : unsigned char, defaults to `|`
   Allows to change the CSV delimiter to a different character
LineStart : ssize_t, defaults to `0`.
   Set to ignore initial number of lines in input file
LineCount : ssize_t, defaults to `-1`
   Set to the number of lines to read.


StringLoader
============

.. note::
    Defined in `src/lib/io/StringLoader.h` and provides *StringHeader*.

StringHeader allows to pass a user-defined std::string to be
used for generating table metadata.


Usage
-----

.. literalinclude:: ../../../src/bin/units_io/string_tests.cpp
    :language: c++
    :linenos: 


Parameters
==========

`StringHeader::params` offers the following parameters:

CSVParams : csv::params
    See discussion of csv::params :ref:`csvparams`

