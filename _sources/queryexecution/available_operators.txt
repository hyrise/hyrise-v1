*******************
Available Operators
*******************

Currently the following plan operators support remote execution

* SimpleTableScan
* ProjectionScan
* HashJoin
* GroupByScan
* TableLoad
* TableUnload
* TableDump

Each operator accepts input and output data of multiple types, e.g. tables, hash maps etc.
Handling this data is implemented in ``OperationalData`` and ``PlanOperation`` classes.

SingleTableLayout
======================

The goal of the **SingleTableLayout** plan operation is to perform a
layout decision for a given workload. Therefore it has multiple
required input parameters. An example of the input is shown below:

.. literalinclude:: ../test/json/simple_layouter_candidate.json
   :language: javascript
   :linenos:

The runtime of the layouter plan operation depends heavily on the
algorithm that is used to find the best layout. There are three
different engines available:


#. ``BaseLayouter`` - Exhaustively iterate over the problem space
#. ``CandidateLayouter`` - Perform early pruning of combinations based on primary partitions
#. ``DivideAndConquerLayouter`` - Before merging candidates, partition the problem space based on an access graph

The layout engine that is used during the execution is set using the
``layouter`` member of the plan operation.

The most important options for the plan operator are setting the
number of rows for the input table (which is used to calcualte the
correct cost for selectivities).

.. literalinclude:: ../test/json/simple_layouter_candidate.json
   :language: javascript
   :linenos:
   :lines: 10

Furthermore it is possible to specify how many possible results the
layouter should return. This option can be used to present multiple
layouts in a UI.

.. literalinclude:: ../test/json/simple_layouter_candidate.json
   :language: javascript
   :linenos:
   :lines: 11

The meta data that is required to generate the best possible layout
are the attributes of the table and the list of operations that is
executed on this table.

.. literalinclude:: ../test/json/simple_layouter_candidate.json
   :language: javascript
   :linenos:
   :lines: 6-9
