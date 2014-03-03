#######
Loading
#######

Hyrise features a loading infrastructure that should make it
sufficiently easy to implement new loaders.


Introduction
============

Currently, Hyrise distinguishes two differnt tasks in loading:

#. Loading metadata
#. Loading data

Naturally some methods of loading may not allow for sensibly
implementing all tasks, as for example distinguishing between metadata
and data is (at the current stage of implementation) not possible for
reading binary data files.


Abstract Overview
=================

A loader has to implement the interface defined in
"src/lib/io/AbstractLoader.h".


.. doxygenclass:: AbstractHeader
    :members:

.. doxygenclass:: AbstractInput
    :members:


How to load data
================

The primary interface for producing a table is

.. doxygenfunction:: Loader::load

it should be used in all cases where a custom combination of input,
header or other available options is needed. Common use cases of
loading can be found in the namespace `Loader::shortcuts` and should be
used unless special needs arise.

If one for example needs to programmatically construct a loader, the
following piece of code should fairly well demonstrate how this could
be done.

.. literalinclude:: ../../src/bin/units_io/loading_example.cpp
    :language: c++
    :linenos: 


Shortcuts
=========

.. doxygenfunction:: Loader::shortcuts::load

.. doxygenfunction:: Loader::shortcuts::loadWithHeader 

.. doxygenfunction:: Loader::shortcuts::loadWithStringHeader


Loaders
=======

.. toctree::

    loading/index
    loading/mysql

