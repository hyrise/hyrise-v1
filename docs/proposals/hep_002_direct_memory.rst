***********************************************
HEP 002: Direct Memory Access for All Operators
***********************************************

Currently most of the access to value ids is performed by calling the
``getValueId()`` method on the pointer to the
``AbstractTable``. However, this has the drawback that if this is
performed millions of times the cost for the virtual method calls
might actually exceed the cost for accessing the different caches.


General Ideas
=============

The overall goal would be to allow pointer based access to the raw
data without having to rely on virtual method calls.

Questions
---------

#. What is the real overhead of virtual method calls? (branching, cycles)
#. How can we achieve a good tradeoff between the higher level
    architecture and the lower level memory access?
#. How can we still be able to support different compression routines? 
#. How much can function pointers help?
#. How much can macros help?
#. Do we need something like pipeline breakers?
#. What is the context that is required to perform the task

   * compression routine
   * table type
#. Do we need to change the executor model to reflect horizontal partitioning?

Proposal
========

Initial Tests
-------------

#. Compare cost for value id based copying with ``memcpy`` based
   copying of data



Implementation
---------------

* we need meta data that enables to abstract from the storage like
  compression, however this meta data must only bee fetched once::

    struct data_meta {
        size_t width = sizeof(value_id_t);
        size_t raw_width = sizeof(value_id_t) * num_cols;
    }

* we need some way to return a pointer to the direct data like::

    void* AbstractTable::getData(size_t col)

  * **problem** when horizontally partitioned, we need to another idea

* based on this pointer there should be a way to advance the pointer
  to a given row, but without having to perform a virtual method call::
  
      data_advance(void* d, size_t n)

  * **problem**: if the table is horizontally partitioned we might
      need another idea

* there should be a way to extract a value from the pointer at the given address::

      value_id_t data_get(void* d)

* there should be a way to set a value id from the pointer at the given address::

      void data_set(void* d, value_id_t val)

  * this goes terribly wrong if we have compressed data, because
    e.g. there is no meta data about how the data is layed out


Data Access Operators
---------------------

The most problematic part is to find a good abstraction from the
different compression routine, run-time and compile-time
information. For Fixed Length implementation this is relatively easy::

    void* fl_data_advance(void* d, size_t offset, void* ctx)
    {
        return ((char*) d) + offset * ((fl_data_meta*) ctx->raw_width);
    }
    
    value_id_t fl_data_get(void* d, void* ctx)
    {
        return *(value_id_t*) d;
    }

Compression might need additional parameters to advance. Basically the
advance and get methods heavily expose implementation logic in an
class external method.


* Can we safely assume that we will always rely on the ``value_id_t``
  data type to identify value ids?


Exemplary implementation for varbit vector::

    struct vb_data_meta
    {
       uint64_t bitmask;
       size_t blockIndex;
       size_t offsetInBlock;
       size_t segmentsInBlock;
    }
    
    
    void* vb_data_advance(void* d, size_t offset, void* ctx)
    {
    
    }
    
    value_id_t vb_data_get(void* d, void* ctx)
    {
    }


Example Plan Operation
----------------------

The following pseudo code describes a sample impelementation for a
plan operation based on direct memory access::


    void SimpleTableScan::executePlanOperation()
    {
        // Evaluate a predicate on three attributes
        void* names[3];
        names[0] = input->getSlice(0);
        names[1] = input->getSlice(1);
        names[2] = input->getSlice(3);
    
    
        // Walk the predicate tree
        size_t i = input->size();
        for (size_t j=0; j < i; ++j)
        {
	     
             if (_operator(names, 3, j)) // runtime, operator should only check no other fun calls
 	     {   
                 /* 
                 There should be two ops, one that supports value 
    	          materialization and one for positions
    	         */
    	     }
	     
	     data_advance(names[0], 1);
    	     data_advance(names[1], 1);
     	     data_advance(names[2], 1);
        }
    
        return result;
    }

In the above plan operation compile-time and run-time code generation
is mixed. The declaration of the ``void*`` array of size ``3`` can
only be decided at runtime, as the increment of each of the
pointers. If the code has only run-time dependencies there would be
two additional for loops: One loop to assign the slices to pointers
and one for loop to increment the pointer position.

Currently all predicates have an ``operator()(size_t row)`` method
that submits the row to the operator. The interface should be changed to::

    class SimpleExpression
    {
        /*...*/
	virtual bool operator()(void* data, size_t len);
    }


