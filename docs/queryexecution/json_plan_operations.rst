.. _jsonplanops:

***************
Plan Operations
***************

Find below a summary of the available Plan Operations and their JSON arguments.

Table Load
==========

Loads a table from file::

    "ID": {
        "type":"TableLoad",
        "table": "table name",
        "filename": "file.data",
        "header": "header.txt",
        "binary": true/false, [optional]
        "unsafe": true/false [optional]
        },




TableUnload
===========

simply unloads a table specified in the ``"table":`` field.

::

    "ID": {
        "type":"TableUnload",
        "table": "table name"
        },




.. _simpletablescan:

SimpleTableScan
===============

Performs a Selection

::

    "ID": {
        "type":"SimpleTableScan",
        "positions": true,
        "predicates": [
            { "type": 7 },
            { "type" : 1, "in" : 0, "f" : "NAME1", "vtype" : 0, "value": 330 },
            { "type" : 1, "in" : 0, "f" : "NAME2", "vtype" : 0, "value": 300 }
            ]
        },

``"positions: true"`` save row positions rather than rows in the intermediate result. [optional. true is default.]

``"predicates:"`` is a list of predicates in prefix order, that will be used to build the predicate tree for selection.

``"type:"`` boolean operators can be given as string or int:
        == ==========
        0   EQ
        1   LT
        2   GT
        3   BETWEEN
        4   COMPOUND
        5   NEG
        6   AND
        7   OR
        8   NOT
        9   MULTI_EQ
        == ==========
        
    ``"in": 0`` simply refers to the fact, that we're performing the selection on the first table. 
    
    ``"f": "NAME1"`` first argument for logical operation. Here "type": 1 (less than).
    
    ``"vtype":`` specifies the type of values in "f"
        ==  ============
        0   IntegerType
        1   FloatType
        2   StringType
        ==  ============
    
    ``"value": value`` second argument for logical operation. e.g. 300
    
    The example given above would build the following predicate (prefix notation): ``OR((NAME1 < 330)(NAME2 < 300))``.




ProjectionScan
==============


Performs a Projection of columns given in ``"fields":``

::

    "ID": {
        "type":"ProjectionScan",
        "fields" : [
        "ADDRNUMBER","NAME_CO","NAME1","NAME2"
        ]
        },
        
``"fields":`` is synonymous to columns/contains a list of all columns to be projected into the result table.




UnionScan
=========


used to combine two table and remove duplicates.

::

    "ID": {
        "type":"UnionScan",
        },




JoinScan
========


currently only performs equi/inner join.

::

    "ID": {
        "type":"JoinScan",
        "join_type":, [optional]
        "predicates": [{
                "type": 3,
                "input_left": 0,
                "field_left": "company_id",
                "input_right": 1,
                "field_right": "employee_company_id"
            }]
        },
        
``"join_type":`` is currently not in use. Right now only inner joins are possible. Outer joins might be implemented at a later time. 
``"predicates":`` multiple predicates may be passed in using logical operators. ("type": 0/1/2 -> see below)

        ``"type": 3,`` specify operator for join condition, here EXP_EQ, i.e. "=" (equi join)
                    == =====
                    0  AND
                    1  OR
                    2  NOT
                    3  EXP_EQ
                    == =====
        
        ``"input_left": 0,`` pass in ID of first table to be joined (e.g. 0)
        
        ``"field_left": "company_id",`` pass in left part of join condition
        
        ``"input_right": 1,`` pass in ID of second table to be joined (e.g. 1)
        
        ``"field_right": "employee_company_id"`` pass in right part of join condition
        
The example given above would perform an inner join on the tables loaded in 0,1 - matching up 0.company_id with 1.employee_company_id.
        




MergeJoin
=========


deprecated.




HashBuild
=========


will build a hash table, hashing the columns specified in "fields".

::

    "ID": {
        "type":"HashBuild",
        "fields": [1]
        },

``"fields": [1]`` pass in column(s) to be hashed, e.g. 1st column.


HashJoinProbe
=============

"probe" other input against hashed table

::

    "ID": {
        "type":"HashJoinProbe",
        "fields": [1]
        },

again ``"fields":`` specifies the column(s) that is to be hash joined.

here's another example of a full HashJoin for clarification. Mind the "edges" connecting HashBuild and Probe.
::
    
    {
        "operators": {
             "-1": {
                    "type": "TableLoad",
                    "table": "reference",
                    "filename": "reference/hash_join_construction_all.tbl"
            },
            "0": {
                "type": "TableLoad",
                "table": "smaller",
                "filename": "tables/hash_table_test.tbl"
            },
            "1": {
                "type": "TableLoad",
                "table": "bigger",
                "filename": "tables/hash_join_construction.tbl"
            },
            "2": {
                "type": "HashBuild",
                "fields" : [0, 1, 2]
            },
            "3": {
                "type": "HashJoinProbe",
                "fields" : [0, 1, 2]
            }
        },
        "edges": [["1", "2"], ["2", "3"], ["0", "3"]]
    }




GroupByScan
===========


group table by attributes specified in ``"fields":``.

::

    "ID": {
        "type": "GroupByScan",
        "fields": ["employee_company_id"],
        "functions": [
            {"type": 1, /*COUNT*/ "field": "employee_company_id"}
            ]
        },
    
``"fields":`` array of fields by which the table is to be grouped

``"functions":`` pass in (aggregate) functions to be performed on the data in "field":

``"type":`` specifies type of aggregate function to be performend.
                == =======
                0  SUM
                1  COUNT
                2  AVERAGE
                == =======
                
``"field":`` field the aggregate function is to be performed on.




MaterializingScan
=================


returns a materialized view that contains the

    ::

        "ID": {
            "type":"MaterializingScan",
            "samples": 3,
            "memcpy": false, [default: false]
            "copyValues": [deprecated]
            },

``"samples": 3`` will output a sample materialized table (here 3 rows).

``"memcpy": false`` will use internal copy by default. set true in order to use getValue().

#TODO: note to Martin: remove "copyValues".




SortScan
========


sorts a table by given attribute(s).

::

    "ID": {
        "type":"SortScan",
        "fields": [0]
        },

``"fields":`` fields/attributes by which the table is to be sorted.




SmallestTableScan
=================


determines the smallest table of all given tables and projects it.

::

    "ID": {
        "type":"SmallestTableScan",
        },




LayoutSingleTable
=================


::

    "0": {
        "type": "LayoutSingleTable",
        "operators": [
        {
            "type": "Select",
            "weight": 1,
            "selectivity": 0.03,
            "attributes": ["employee_id"]
        }
        ],
        "attributes": ["employee_id", "employee_company_id", "employee_name"],
        "num_rows": 1000,
        "layouter": "BaseLayouter"
    },


``"operators":``

``"type":``
``"weight":``
``"selectivity":``
``"attributes":``

``"attributes":``

``"num_rows":``

``"layouter":``




LayoutTableLoad
===============


::

    "1": {
        "type": "LayoutTableLoad",
        "table": "emplyoees",
        "filename": "tables/employees.data",
        "input_row": 3
    }




MetaData
========

Returns either a list of the loaded tables or if specified, the columns with their 
corresponding datatypes of each input table.

A simple example query to get the table list:

::

    {
        "operators": {
            "meta" : {
                "type" : "MetaData"
            }
        }
    }
    

And another example query to fetch the column info for some tables, which are loaded beforehand. 
If the tables are already loaded, specifying them in the input field would suffice to 
get their column info.

.. literalinclude:: ../test/autojson/meta_data.json
    :language: javascript
    :linenos:
