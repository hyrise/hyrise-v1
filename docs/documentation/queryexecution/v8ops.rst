.. _v8ops:

#####################
Integrating Google V8
#####################

Hyrise comes with the possibility to implement plan operations in JavaScript
instead of C++ to ease the prototypical development. Of course the performance
of these plan operations will not reach a highly optimized C++ code, but can
help to asses the algorithmic performance of sample operations.

This document gives an overview how the integration between V8 and HYRISE is
achieved and how the interface between JavaScript and C++ is defined.


Using JavaScript for Plan Operations
====================================

Googles V8 is integrated as a separate and independent component that will be
included and linked during compile time. Currently, we propose to download the
most recent version of V8 from their website and compile manually. To enable
JavaScript plan operations you have to set the following variables in the
``settings.mk`` file::

	USE_V8 = 1
	V8_BASE_DIRECTORY=path/to/v8/base

Now, the ScriptPlan operation is enabled and allows to write new plan
operations in JavaScript.

Once you started HYRISE you can call plan operations that are stored in plain
text JavaScript files that are available to the server. The environment
variable ``HYRISE_SCRIPT_PATH`` determines the base folder where the server
expects to find those files. If the variable is not set, int points to the
directory from where the server was launched.


Defining a new Plan Operation
-----------------------------

The minmal plan operation defines a script that defines the following function::

	function hyrise_run_op(input) {
	}

The variable ``input`` defines an array that contains all references to input
tables of the plan operation. The requirement is that the function will return
a single table as output.


API for the Plan Operation
--------------------------

In general, the idea is to offer the same API that is known from the
``AbstractTable`` interface to the JS side. However, there are some
limitations to this approach. The biggest problem is creating lot's of
intermediate results is very time consuming and will break the advantage of
using JS.

The most important operations on a table are:

* ``getAttributeVectors()`` - equivalent to the version of the AbstractTable.
   Intended to directly access the underlaying storage of the table.
* ``valueId(col, row)`` - returns the value ID struct with valueId and tableId
* ``ValueIdV(col, row)`` - only returns the value ID value
* ``resize(num)`` - resize the table if possible, throw exception otherwise
* ``setValueInt(col, row val)``
* ``setValueFloat(col, row val)``
* ``setValueString(col, row val)``
* ``getValueInt(col, row val)``
* ``getValueFloat(col, row val)``
* ``getValueString(col, row val)``

The following other functions are globally available:

* ``copyStructureModifiable(table)``
* ``createPointerCalculator(table, fields)``
* ``buildVerticalTable(tab, tab)`` - concatenate two tables and build a vertical table
* ``include(file)`` - include a file and make all functions globally available
* ``log(string)`` - log string to the HYRISE logging facilities

To create a custom output table the global function ``buildTable(spec,
groups)`` should be used. A simple example to build a table would be::

    buildTable([
		{"type": "INTEGER", "name": "company_id"},
		{"type": "STRING", "name": "company_name"}
		],[2]);

This example creates a two column table with the given attributes named by the
first argument and the number of groups identified by the second parameter
array.
