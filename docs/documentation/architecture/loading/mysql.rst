############
MySQL Loader
############

.. note::
    MySQLInput is only available when compiled with WITH_MYSQL as
    described in :doc:`../../developing/build_environment`.

.. _sqldep:

Additional dependencies
=======================

The following dependencies are needed once WITH_MYSQL has been activated:

* libmysqlcppconn-dev
* libmysqlclient-dev


Environment Variables
=====================

The following environment variables can be used to influence the
default initialization values of *MySQLInput*:

* HYRISE_MYSQL_HOST
* HYRISE_MYSQL_PORT
* HYRISE_MYSQL_USER
* HYRISE_MYSQL_PASS

Yet, for specific implementations of functionality, one might rather
want to use the parameters defined with `MySQLInput::params()`.


Parameters
==========

Connection settings:

Host : string 
    * defaults to environment variable `HYRISE_MYSQL_HOST`
    * otherwise falls back to `127.0.0.1`
    

Port : string
    * defaults to environment variable `HYRISE_MYSQL_PORT`
    * otherwise falls back to `3306`

User : string
    * defaults to environment variable `HYRISE_MYSQL_USER`
    * otherwise falls back to `root`

Pass : string
    * defaults to environment variable `HYRISE_MYSQL_PASS`
    * otherwise falls back to `root`

Import settings:

Schema : string
    * set for table schema

Table : string
    * set for table to import


Usage
=====

.. literalinclude:: ../../../src/bin/units_io/mysql_tests.cpp
    :linenos:


Implementation details
======================

In order to translate MySQL types to HYRISE types, in
`MySQLLoader.cpp` defines a data structure named `translations`. It
holds all the information necessary for the loader to translate a
column from MySQL to HYRISE, namely maps MySQL column names to HYRISE
column types and conversion functions. Imports will fail unless all
columns can be properly converted, thus when you encounter a type not
yet covered by translations, either open a ticket or fix it by
providing your own method.
