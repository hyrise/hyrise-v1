.. HYRISE documentation master file, created by
   sphinx-quickstart on Tue Aug 16 11:42:12 2011.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to HYRISE's documentation!
==================================

What is HYRISE
--------------

HYRISE is a main memory database
prototype that expects to deliver significant performance improvement
due to the possibility to vertically partition any table. In this Wiki
we try to collect all relevant information that is required to
understand the architecture and extend it.


Traditional databases are separated into ones for current data from
the day-to-day business processes and ones for reporting and
analytics. For fast moving businesses moving data from one silo to
another is cumbersome and takes too much time. As a result the new
data arriving in the reporting system is already old by the time it is
loaded. HYRISE proposes a new way to solve this problem: It analyzes
the query input and reorganizes the stored data in different
dimensions.

In detail, HYRISE partitions the layout of the underlaying tables in a
vertical and horizontal manner depending on the input to this layout
management component. The workload is specified as a set of queries
and weights and is processed by calculating the layout dependent costs
for those queries. Based on our cost-model we can now calculate the
best set of partitions for this input workload. This optimization
allows great speed improvements compared to traditional storage
models.

Contents:

.. toctree::
   :glob:
   :maxdepth: 2

   getting_started
   architecture   
   query_execution
   developing
   faq
   proposals

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

