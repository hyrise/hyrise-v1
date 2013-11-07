============
JSON Queries
============
.. _qexec:

JSON Query Execution
********************

#. Set up where to look for table data directory (folder already exists) ::

      cd hyrise
      export HYRISE_DB_PATH=`pwd`/test

#. Export library directory ::

      export LD_LIBRARY_PATH=./build/:$LD_LIBRARY_PATH

#. Starting the server :: 

      ./build/hyrise_server -l ./build/log.properties -p 5000

#. Now in a different terminal, you can send a query to the server ::

       curl -X POST --data-urlencode query@test/gettingstarted.json --data performance=true
       http://localhost:5000/jsonQuery

Details
*******

JSON Queries are a different representation of the original flow graph
of the query plan. Basically a k-v map defines all operators that are
connected using edges. An example for a simple query plan is defined
below ::

    {
        "papi": "PAPI_L1_DCA",
        "operators" : {
            "0": {
                "type": "LoadFile",
                "filename" : "tables/companies.tbl"
            },
            "1" : {
                "type" : "SimpleTableScan",
                "positions" : true,
                "predicates":[
                { "type": 7 },
                { "type" : 2, "in" : 0, "f" : "company_id", "vtype" : 0, "value": 2 },
                { "type" : 0, "in" : 0, "f" : "company_name", "vtype" : 2, "value": "Microsoft" }
                ]
            },
            "2" : {
                "type" : "ProjectionScan",
                "fields" : [
                "company_id","company_name"
                ]
                }
            },
            "edges": [["0","1"],["1","2"]]
        }

The HYRISE HTTP interface accepts additional parameters through data
params, in this example, we set `performance` as `true` so we will
receive extra performance data in the response. Any other value for
`performance` (or no value at all) will lead to performance data not
being send back to the client.

Parameters
**********

The JSON input document defines 3 main input keys

#. ``papi`` - Any valid PAPI event name that is available on this  machine. Currently there are always two events captured during the  plan execution ``PAPI_TOT_CYC`` and the additional event.
#. ``operators`` - A map of keys and values representing the different operators. The key is later referenced to perform dependency detection.
#. ``edges`` - The edges define the actual flow graph. Dependencies are listed as pairs of operators. There is only one special case  where only one plan operator is given, than a circular edge is given  ``["0", "0"]``.

The edges of the flow graph may describe any non-circular graph with the restriction that any vertice may have multiple inputs, but only a single output.

With this particular JSON Query, Hyrise Server would perform three Database Operations. 
First on the "Edge" ["0","1"] a table is being loaded into the database from file (operator: "0"). A SimpleTableScan is then being performed on that table (operator: "1"), giving predicates for the selection in prefix notation. The example above would translate to: (company_id > 2) OR (company_name = "Microsoft"). See :ref:`simpletablescan` for more details on the SimpleTableScan operation.

A detailed Overview of all available Database operators and their respective JSON notation can be found at :ref:`jsonplanops`.

The second edge ["1","2"] will make Hyrise perfom a Projection on the result of the last edge. The syntax is quite straightforward here - you simply pass a list of all the columns to be projected. That would be "company_id" and "company_name" in this example.

To get a feel for json Query execution, go ahead and run the query above -> see :ref:`qexec` below.

Your response from the server should look something like this::

    {
    	"header" : [ "company_id", "company_name" ],
    	"performanceData" : [
    		{
    			"data" : 0,
    			"duration" : 69,
    			"endTime" : 2.6423050,
    			"executingThread" : "0x7fcef2d006d0",
    			"id" : "0",
    			"name" : "TableLoad",
    			"papi_event" : "PAPI_L1_DCA",
    			"startTime" : 2.5683790
    		}, 
                ...
    	],
    	"rows" : 
    	[
    		[ 2, "Microsoft" ],
    		[ 3, "SAP AG" ],
    		[ 4, "Oracle" ]
    	]
    }


``"header"`` outputs the header for result table (=list of field names).

``"performanceData"`` gives detailed performance data.

    More specifically it returns performance data on each one the Plan Operations. Here: ``"name":"TableLoad"``, ``"name":"SimpleTableScan"`` and ``"name":"ProjectionScan"``.
    
    ``"id":`` and ``"name":`` are used respectively.
    
    ``"data":`` returns event counter for the measured PAPI event.
    
    ``"duration":`` refers to actual clock cycles required to run the operation.
    
    ``"startTime":`` and ``"endTime":`` give the start and end time of the operation in nanoseconds.
    
    ``"papi_event":`` specifies which Papi Event was used to measure performance.
    
    Additionally there is performance data available for the parsing of the JSON Query -> ``"name":"RequestParseTask"`` as well as for outputting the response -> ``"name":"ResponseTask"``.

``"rows"`` gives a list of the rows resulting from the query.


