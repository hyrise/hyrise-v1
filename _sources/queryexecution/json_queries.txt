************
JSON Queries
************

JSON Queries are a different representation of the original flow graph
of the query plan. Basically a k-v map defines all operators that are
connected using edges. An example for a simple query plan is defined
below::

    { 
        "papi": "PAPI_L1_DCA",
        "operators" : {
            "0": {
                "type": "TableLoad",    
                "table": "ADRC",
                "header": "ADRC_header_1.txt",
                "filename" : "ADRC.data" 
            },
            "1" : {
                "type" : "SimpleTableScan",
                "positions" : true,
                "predicates":[
                { "type": 7 },
                { "type" : 1, "in" : 0, "f" : "NAME1", "vtype" : 0, "value": 330 }, 
                { "type" : 1, "in" : 0, "f" : "NAME2", "vtype" : 0, "value": 300 }
                ]
            },
            "2" : {
                "type" : "ProjectionScan",
                "fields" : [
                "ADDRNUMBER","NAME_CO","NAME1","NAME2" 
                ]
            }
        },
        "edges": [["0","1"],["1","2"]]
    }

The result of the above query is again a JSON document that contains
the result table and additional information::
    
       "header" : [ "ADDRNUMBER", "NAME_CO", "NAME1", "NAME2" ],
       "message_handling" : 
       {
           "parse_request" : 679,
           "respond_request" : 979
       },
       "performanceData" : 
       [
    
           {
               "data" : 0,
               "duration" : 428,
               "name" : "TableLoad",
               "papi_event" : "PAPI_L1_DCA" 
           },
    
           {
               "data" : 0,
               "duration" : 976,
               "name" : "SimpleTableScan",
               "papi_event" : "PAPI_L1_DCA" 
           },
    
           {
               "data" : 0,
               "duration" : 10,
               "name" : "ProjectionScan",
               "papi_event" : "PAPI_L1_DCA" 
           }
       ],
       "rows" : 
       [
           [ 4158, 2973, 2086, 6 ],
           [ 4371, 4045, 230, 2748 ],
       ]
    
Parameters
===========

The JSON input document defines 3 main input keys

#. ``papi`` - Any valid PAPI event name that is available on this  machine. Currently there are always two events captured during the  plan execution ``PAPI_TOT_CYC`` and the additional event
#. ``operators`` - A map of keys and values representing the different  operators. The key is later refrenced to perform dependency detection 
#. ``edges`` - The edges define the actual flow graph. Dependencies are listed as pairs of operators. There is only one special case  where only one plan operator is given, than a circular edge is given  ``["0", "0"]``.

The edges of the flow graph may describe any non-circular graph with
the restriction that any vertice may have multiple inputs, but only a
single output.

Settings
===========

Currently, there is no designated scheduling unit implemented in HYRISE. To simulate scheduling or other decisions, a settings operation can be executed to set certain options::

	"ID": {
		"type": "SettingsOperation",
		"threadpoolSize": 2
		[, your additional options...]
	}

Options can be defined in the Settings data container using SettingsOperation. Use and/or implement additional operations to apply or set and apply them, like the ThreadpoolAdjustment operation::

	"ID": {
		"type": "ThreadpoolAdjustment",
		"size": 2
	}

Executing this operation will set threadpoolSize in Settings to 2 and instantly apply it on the boost threadpool.