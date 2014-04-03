#############
Group By Scan
#############


Usage
======

For implementing the group by scan we use hashing. Therefore, it is necessary that
every GroupBy instance has the source table itself, a hash map, the grouping fields
as well as possible grouping functions (sum or count) as input.

The interface for this PlanOperation is straight-forward:

Example:

.. code-block:: c
  :linenos:

    StorageManager *sm = StorageManager::getInstance();

    AbstractTable *source = sm->getTable("stock_level");

    GroupByScan gs;
    gs.addInput(source);

    //third column as grouping field 
    gs.addField(3);

    HashBuild hs;
    hs.addInput(source);

    //third column is grouping field - hash it
    hs.addField(3);

    AbstractHashedColumn *group_map = hs.execute()->getResultHashedColumn();
    
    gs.addInput(group_map);

    AbstractTable *result = gs.execute()->getResultTable();

    result->release();
    group_map->release();
    source->release();


If more than one column is part of the grouping expression, just add more fields to the the GroupByScan instance and HashBuild-Operation.
The implementation of *HashBuild* and *HashedColumn* takes care of building the correct hash group.


executePlanOperation()
======================

Commented Code:

.. code-block:: c
  :linenos:

    /* the result table layout is created with the help of the
     * added grouping fields and attached aggregate functions
     */

    AbstractTable *resultTab = createResultTableLayout();
    AbstractHashedColumn *groupResults = getInputHashedColumn();
     
    AbstractTable *input = getInputTable(0);
    
    /* if there are grouping fields there has to be a hash map in the input
     * else just consider attached aggregate functions
     */

    if(_field_definition.size() != 0 && groupResults != NULL)
    {
        /* starting with first group */
        size_t row = 0;
        
        /* get hash map iterator from groupResults input
         * iterating over all available hash keys
         * each key represents a single group 
         */

        for(AbstractHashedColumn::map_const_iterator_t hit = groupResults->getMapBegin(); hit != groupResults->getMapEnd(); ++hit)
        {
            /* For each columnNr in added grouping fields
             * write its grouping result to the result table
             */

            BOOST_FOREACH(field_t columnNr, _field_definition)
            {
                /* To avoid excessive use of switch case statements
                 * we use a template based type switch (src/lib/storage/meta_storage.h)
                 * this will call the correct typed setValue method on resultTab
                 * and writes the result in the row represented by the "row" variable
                 */

                hyrise::storage::write_group_functor<int> fun(input, resultTab, hit, (size_t)columnNr, row);
                hyrise::storage::type_switch<hyrise_basic_types> ts;
    
                ts(input->typeOfColumn(columnNr), fun);
            }
    
            /* If there are attached aggregate functions (count or sum)
             * we execute the function but only consider the rows that
             * build the identified group ("hit->second").
             * processValuesForRows is implemented in AggregateFunctions.h
            */

            BOOST_FOREACH(AggregateFun *funct, aggregate_functions)
            {
                funct->processValuesForRows(input, hit->second, resultTab, row);
            }

            /* proceed with next group */
            row++;
        }
    }
    else
    {
        /* There are no grouping fields, we only need to execute
         * all attached aggregate functions (if any).
         * processValuesForRows will execute the function on all rows
         * if second parameter is NULL
        */

        BOOST_FOREACH(AggregateFun *funct, aggregate_functions)
        {
            funct->processValuesForRows(input, NULL, resultTab, 0);
        }
    }
     
    /*adding the result to output*/
    this->addResult(resultTab);

Aggregate Scan (Count, Sum, Average, Min, Max)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

HYRISE supports these different aggregate functions so far:

- *SumAggregateFun* calculates the sum of a given float or integer column.
- *CountAggregateFun* counts the values in a given column (distinct counting is possible).
- *AverageAggregateFun* calculates the average value of a given float of integer column.
- *MinAggregateFun* retrieves the minimum value of a given column.
- *MaxAggregateFun* retrieves the maximum value of a given column.

*processValuesForRows* can be called in two different ways.
In AggregateFunctions.h::
    
   virtual void processValuesForRows(AbstractTable *t, pos_list_t *rows, AbstractTable *target, size_t targetRow) = 0;

The first is to provide a *pos_list_t vector* in order to take a subset of all values into account.
If this parameter is *nullptr* the given *AggregateFun* will compute the result using
all values.

The result is directly written to *target* in row *targetRow*. *AbstractTable *t* is the input table of the calling
GroupByScan instance.

The resulting column may be given any name using  *columnName()*. If no name is set the default name will be used
which looks like *[SUM/COUNT/AVG..](columnName)* (e.g. *MAX(amounts)*).

*CountAggregateFun* may be set to distinct by *setDistinct()* so that the number of different field values will be
counted. The default is non-distinct counting.

Parallelization
===============

In order to parallelize the group by scan following the HYRISE typical parallelization approach (input distribution)
we introduced two new classes: *AbstractHashTable* and *HashTableView*. *HashTableView* and *HashTable* both inherit
from *AbstractHashTable*.

The *HashTableView* subclass maps only a range of key value pairs of its underlying HashTable for an easy splitting.

Similar to *AbstractTable::view* the AbstractHashTable implements a view method returning a new HashTableView
as described above::
    
     AbstractHashTable *AbstractHashTable::view(size_t first, size_t last)
     {
        return new HashTableView(this, first, last);
     }

The parameters first and last are computed in GroupByScan::splitInput()::
  
    void GroupByScan::splitInput()
    {
        hash_table_list_t &hashTables = input.getHashTables();
        if (_count > 0 && !hashTables.empty())
        {
           u_int64_t first, last;
           distribute(hashTables[0]->size(), first, last);
           replace(hashTables.begin(), hashTables.begin()+1, hashTables[0], hashTables[0]->view(first, last+1));
        }
    }

If there is more than one GroupByScan instance and a HashTable is available (since it is mandatory for our GroupByScan)
we calculate a distribution for a given GroupByScan instance and copy its begin and end to the variables first and last.
After that, we replace the original complete HashTable in the operations input by a new *HashTableView* providing the
correct iterator pair for a given instance. After all instances have completed their grouping tasks the HYRISE
QueryTransformationEngine automatically attaches a UnionScan writing the results of all instances into one table.

A parallel GroupByScan can be executed with the HYRISE JSON interface as follows:


.. literalinclude:: ../../../test/autojson/groupby.json
    :language: json
    :linenos:


The result is a parallel execution of a GroupBy operation resulting in a table describing
the number of employees per company.

Global Aggregation
^^^^^^^^^^^^^^^^^^

This group by implementation works only well if either main or delta are
empty. To alleviate the problem and allow global aggregation it is possible to
perform value based aggregation instead of value IDs.

This is achieved by manually setting the hash algorithm to values (as used in
the traditional join) and explicitly setting the aggregation type to global.
An example is shown in the following test-case:

.. literalinclude:: ../../../test/autojson/groupby_global.json
    :language: json
    :linenos:

