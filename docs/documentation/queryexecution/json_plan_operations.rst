.. _jsonplanops:

###############
Plan Operations
###############

Find below a summary of the available Plan Operations and their JSON arguments. 
Each operator accepts input and output data of multiple types, e.g. tables, hash maps etc.
Handling this data is implemented in ``OperationalData`` and ``PlanOperation`` classes.

    - :ref:`tableLoad`
    - :ref:`tableUnload`
    - :ref:`simpleTableScan`
    - :ref:`projectionScan`
    - :ref:`insertScan`
    - :ref:`unionScan`
    - :ref:`joinScan`
    - :ref:`mergeJoin`
    - :ref:`createIndex`
    - :ref:`indexScan`
    - :ref:`hashBuild`
    - :ref:`hashJoinProbe`
    - :ref:`groupByScan`
    - :ref:`materializingScan`
    - :ref:`sortScan`
    - :ref:`smallestTableScan`
    - :ref:`layoutSingleTable`
    - :ref:`layoutTableLoad`
    - :ref:`noOp`
    - :ref:`distinct`


.. _tableLoad:

Table Load
==========

This operator loads a table from a file::

    "load": {
        "type":"TableLoad",
        "table": "table name",
        "filename": "file.data",
        "header": "header.txt",
        "binary": true/false, [optional]
        "unsafe": true/false [optional]
        },


.. _tableUnload:

Table Unload
============

Simply unloads a table specified in the ``"table":`` field.

::

    "unload": {
        "type":"TableUnload",
        "table": "table name"
        },


.. _simpleTableScan:

Simple Table Scan
=================

This operator performs a Selection.

::

    "scan": {
        "type":"SimpleTableScan",
        "positions": true,
        "predicates": [
            { "type": 7 },
            { "type" : 1, "in" : 0, "f" : "NAME1", "vtype" : 0, "value": 330 },
            { "type" : 1, "in" : 0, "f" : "NAME2", "vtype" : 0, "value": 300 }
            ]
        },

Use ``"positions: true"`` to save row positions rather than rows in the intermediate result. This is optional and by default true.

``"predicates:"`` is a list of predicates in prefix order, that will be used to build the predicate tree for selection.

    For each predicate you can specify the operator type, the input table, the data types and a value for logical operations.

    Use ``"type:"`` to specify the boolean operators. They can be given as string or int:
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
        16  LIKE
        == ==========
        
    The ``LIKE`` expression expects a regular expression for regex ``("a*b.b(ca)*" matches "aaabwbcaca")``

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


.. _projectionScan:

Projection Scan
===============

Performs a Projection of columns given in ``"fields":``

::

    "projection": {
        "type":"ProjectionScan",
        "fields" : [
            "ADDRNUMBER","NAME_CO","NAME1","NAME2"
            ]
        },
        
``"fields":`` is synonymous to columns/contains a list of all columns to be projected into the result table.


.. _insertScan:

Insert Scan
===========

Performs an Insert into a given table. The ``"data":`` attribute takes an array with the rows to be inserted.

::

    "insert" : {
            "type" : "InsertScan",
            "data" : [
                [2009,1,2000],
                [2009,2,2500],
                [2009,3,3000],
                [2009,4,4000]
            ]
        }


.. _unionScan:

Union Scan
==========

The Union Scan combines two tables and removes duplicates.

::

    "union": {
        "type":"UnionScan",
        },


.. _joinScan:

Join Scan
=========

This operator performs a Join. It currently only performs equi/inner join.

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


.. _mergeJoin:

Merge Join
==========

deprecated.


.. _createIndex:

Create Index
============

This operator can be used to add an index to a table.

::

    "index": {
      "type": "CreateIndex",
      "fields": ["order_id"],
      "index_name": "vbak_order_ix"
    }


.. _indexScan:

Index Scan
==========

This operator initiates a table scan using a given index.

::

    "index": {
      "type": "IndexScan",
      "vtype": 0,
      "value": 3,
      "fields": ["employee_company_id"],
      "index": "emp_comp_ix"
    }


.. _hashBuild:

Hash Build
==========

This one will build a hash table, hashing the columns specified in "fields".

::

    "hash": {
        "type":"HashBuild",
        "fields": [1]
        },

``"fields": [1]`` pass in column(s) to be hashed, e.g. 1st column. Depending
on the hashing type it is possible to supply the parameter
``"key":"join|groupby"`` that is used to decide if hashing is based on values
or value IDs. Please be aware that for aggregation on multiple horizontal
partitions it is required to use key type ``join``.


.. _hashJoinProbe:

Hash Join Probe
===============

"probe" other input against hashed table

::

    "hashProbe": {
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


.. _groupByScan:

GroupBy Scan
============

This scan groups a table by attributes specified in ``"fields":`` using aggregation functions specified in ``"functions":``. This oeprator only works on hash tables, so perform a :ref:`hashBuild` before.

::

    "group": {
        "type": "GroupByScan",
        "fields": ["employee_company_id"],
        "functions": [
            {"type": 1, /*COUNT*/ "field": "employee_company_id", "distinct": true}
            ]
        },
    
``"fields":`` array of fields by which the table is to be grouped

``"functions":`` pass in (aggregate) functions to be performed on the data in "field":

``"type":`` specifies type of aggregate function to be performend.
                == =======
                0  SUM
                1  COUNT
                2  AVERAGE
                3  MIN
                4  MAX
                == =======
                
``"field":`` field the aggregate function is to be performed on.

``"distinct":`` [only for COUNT] determines whether to count distinct. [optional. default is false]

If aggregation is executed on multiple horizontal partitions it is required to
use a special argument called ``"key"`` that expects a hash table build on
values instead of value IDs.

::

    "ID": {
        "type": "GroupByScan",
        "fields": ["employee_company_id"],
        "key" : "value",
        "functions": [
            {"type": 1, /*COUNT*/ "field": "employee_company_id", "distinct": true}
            ]
        },

.. _materializingScan:

Materializing Scan
==================

The Materializing Scan returns a materialized view that contains the

::

    "materializing": {
        "type":"MaterializingScan",
        "samples": 3,
        "memcpy": false, [default: false]
        "copyValues": [deprecated]
        },

``"samples": 3`` will output a sample materialized table (here 3 rows).

``"memcpy": false`` will use internal copy by default. set true in order to use getValue().

#TODO: note to Martin: remove "copyValues".


.. _sortScan:

Sort Scan
=========

This operator can be sorted a table by given the attribute(s).

::

    "sort": {
        "type":"SortScan",
        "fields": [0]
        },

``"fields":`` fields/attributes by which the table is to be sorted.


.. _smallestTableScan:

Smallest Table Scan
===================

This scan determines the smallest table of all given tables and projects it.

::

    "smallest": {
        "type":"SmallestTableScan",
        },


.. _layoutSingleTable:

Layout Single Table
===================

The goal of the **SingleTableLayout** plan operation is to perform a
layout decision for a given workload. Therefore it has multiple
required input parameters. An example of the input is shown below:

.. literalinclude:: ../../test/json/simple_layouter_candidate.json
   :language: javascript

The runtime of the layouter plan operation depends heavily on the
algorithm that is used to find the best layout. There are three
different engines available:

#. ``BaseLayouter`` - Exhaustively iterate over the problem space
#. ``CandidateLayouter`` - Perform early pruning of combinations based on primary partitions
#. ``DivideAndConquerLayouter`` - Before merging candidates, partition the problem space based on an access graph

The layout engine that is used during the execution is set using the
``layouter`` member of the plan operation.

The most important option for the plan operator is setting the
number of rows for the input table, which is used to calcualte the
correct cost for selectivities.

.. literalinclude:: ../../test/json/simple_layouter_candidate.json
   :language: javascript
   :lines: 10

Furthermore you can specify how many possible results the
layouter should return. This option can be used to present multiple
layouts in a UI.

.. literalinclude:: ../../test/json/simple_layouter_candidate.json
   :language: javascript
   :lines: 11

The meta data that is required to generate the best possible layout
are the attributes of the table and the list of operations that is
executed on this table.

.. literalinclude:: ../../test/json/simple_layouter_candidate.json
   :language: javascript
   :lines: 6-9


.. _layoutTableLoad:

Layout Table Load
=================

::

    "layout": {
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

.. _noOp:

NoOp
====

This operator does not perform any action and can be used to drop intermediate results.

:: 

    "noop": {
        "type": "NoOp"
    }


.. _distinct:

Distinct
========

The distinct operator returns a table with only distinct values in the specified fields.

::
    
    "distinct": {
      "type": "Distinct",
      "fields": ["employee_company_id"]
    }
