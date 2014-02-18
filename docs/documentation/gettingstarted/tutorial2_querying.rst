####################
Querying with Hyrise
####################
.. _qexec:


JSON Queries
============

At the moment Hyrise only accepts queries in Javascript Object Notation. These JSON Queries are a different representation of the original flow graph of the query plan. Basically a k-v map defines all operators that are connected using edges. For more details on how to write queries and how they are processed go to :doc:`../queryexecution/json_queries` and check :ref:`jsonplanops` for a list of available operators. An example for a simple query plan is defined
below:

.. code-block:: js
  :linenos:

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


Query Execution
===============

To send these queries to Hyrise, after starting the Server (:doc:`tutorial1_starting_the_server`), you can post them as JSON-String (usually from a file) via the HTTP interface to Hyrise ::

       curl -X POST --data-urlencode "query@test/gettingstarted.json" --data "performance=true" http://localhost:5000/jsonQuery

