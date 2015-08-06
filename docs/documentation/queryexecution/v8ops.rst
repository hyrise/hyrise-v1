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
included and linked during compile time.

V8 can be activated by adding::

    WITH_V8 := 1

to ``settings.mk``. This will result in the automated download and build of the
appropriate version of V8.

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
* ``setValueInt(col, row, val)``
* ``setValueFloat(col, row, val)``
* ``setValueString(col, row, val)``
* ``getValueInt(col, row)``
* ``getValueFloat(col, row)``
* ``getValueString(col, row)``

The following other functions are globally available:

* ``copyStructureModifiable(table)``
* ``createPointerCalculator(table, fields)``
* ``buildVerticalTable(tab, tab)`` - concatenate two tables and build a vertical table
* ``buildTableShort(spec, [size])`` - alternative, shorter way to create a table
* ``buildTableColumn(spec, [size])`` - create a column store
* ``include(file)`` - include a file and make all functions globally available
* ``log(string)`` - log string to the HYRISE logging facilities
* ``executeQuery(query, [inputTable], [paramsForPreparedStatements])`` - execute a query in Hyrise similar as known from ordinary JSON queries for hyrise
  * query: the query plan in JSON as usual
  * inputTable: a table used as input for the first planop of the query
  * paramsForPreparedStatements: parameters used for something similar as prepared statements. If the query contains masks with the following
format #{nameOfKey} and the object passed as paramsForPreparedStatemnts contains a key ``nameOfKey`` the mask is replaced by the corresponding
value of the object.

To create a custom output table the global function ``buildTable(spec,
groups)`` should be used. A simple example to build a table would be::

	buildTable([
		{"type": "INTEGER", "name": "company_id"},
		{"type": "STRING", "name": "company_name"}
		],[2]);

This example creates a two column table with the given attributes named by the
first argument and the number of groups identified by the second parameter
array.

An easier way of doing something similar, but without the possibility to influence
the partitioning is the global function ``buildTableShort(spec, [size])``. For example::

	buildTableShort({
		"id": "INTEGER",
		"name": "STRING"
	}, 5);

This example creates a table with the two columns ``id`` and ``name``. The
table has 5 rows. The function ``buildTableColumn(spec, [size])`` works the
same way, but creates a column store. (each column is a seperate table)

API for retrieving, storing and executing stored Javascript procedures
----------------------------------------------------------------------

The endpoint ``/JSProcedure/`` delivers an interface for retrieving, storing
and executing stored Javascript procedures. The interface expects the orders in
a JSON-encoded object which is sent with the ``procedure`` field of the request body.

The following actions are available::

	get (no parameters) - returns a list of all stored procedures currently available
	get (procedureName) - returns content of procedure with @procedureName

	create (procedureName, procedureSource) - stores @procedureSource under the name @procedureName. returns OK

	delete (procedureName) - deletes the procedure with the name @procedureName. returns OK

	execute (procedureName) - returns result of executing stored procedure @procedureName
	execute (procedureSource) - returns result of executing @procedureSource

	papiEventsAvailable (no parameters) - returns a list of all available papi events

A simple example for retrieving the list of all currently stored JS procedures::

	data = {
		action: "get"
	};

	$.ajax({
		type: "POST",
		url: "http://localhost:5000/JSProcedure/",
		data: "procedure="+encodeURIComponent(JSON.stringify(data)),
		success: function(data) {
			console.log(data);
		}
	});

It is also possible to provide parameters to a stored procedure. The first parameter is always
the input table. It is obligatory to provide this one in the javascript source code, but it cannot
be set via the JSON query information. All other parameters and their types can be choosen freely.
An example procedure could look like this:

	function hyrise_run_op(input, myParamNumber, myParamString) {
		var value = myParamNumber + 5;
		log(value + " myParamString");
	}

To populate the parameters with values the ``parameter`` field of the request body should be set
like the following:

	curl -X POST --data-urlencode "procedure={\"action\":\"execute\",\"procedureName\":\"test\"}" --data-urlencode "parameter=[1,\"peter\"]" http://server/JSProcedure/


Retrieving performance data
---------------------------

If the field ``performance`` of the request body is set to true, a set of performance data is
sent with the response object. The key ``performanceData`` contains an array of objects. In our
case only the first object with the ``name: StoredProcedureExecution`` is of interest. This very
object has a key called ``subQueryPerformanceData`` which contains an array for each occurence
(line number) of ``executeQuery`` in the source code of the stored procedure. This array holds
performance objects for each planoperation that is part of this query. Metrics are usually recorded
as CPU cycles, but for the top operation ``StoredProcedureExecution`` in nanoseconds.

Please note that the cardinality cannot be provided for all kinds of planoperations, some of them
do not return a table hence there is no cardinality. If this is the case the value of max(size_t)
is sent.

An example might clear this up::

	{
		"header": [
			"company_id",
			"company_name"
		],
		"performanceData": [{
			"duration": 124056844,
			"id": "__StoredProcedureExecution",
			"name": "StoredProcedureExecution",
			...,
			"subQueryPerformanceData": {
				"50": [
					{
						"duration": 65796,
						"id": "filterA",
						"name": "SimpleTableScan",
						"papi_event": "PAPI_TOT_INS",
						"cardinality": 123342
						...
		  			},
		  			{
						"duration": 35680,
						"id": "filterB",
						"name": "SimpleTableScan",
						"papi_event": "PAPI_TOT_INS",
						"cardinality": 64323
						...
		 			}
				],
				"63": [
					{
						"duration": 32318,
						"id": "filterC",
						"name": "SimpleTableScan",
						"papi_event": "PAPI_TOT_INS",
						"cardinality": 53950
						...
					}
				]
			}
		}]
	}
