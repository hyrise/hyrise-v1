########
HashJoin
########

HYRISE' HashJoin implementation is divided into a build and probe phase.


Build Phase: HashBuild
======================

For the first phase, the build phase, the HashBuild PlanOperation is used. It accepts the first AbstractTable and a list of fields as input to produce a HashTable:

.. code-block:: c
  :linenos:

	void HashBuild::executePlanOperation()
	{
		addResult(new HashTable(
			getInputTable(),
			new field_list_t(_field_definition)));
	}
                                                                                                                                                 
	HashTable(AbstractTable *table, field_list_t *fields)
	{
		for (pos_t row = 0; row < table->size(); ++row)
		{
			add(hashFor(table, *fields, row), row);
		}
	}                                                                                                                                                    

To build the HashTable, the cell values of all table's rows in the specified fields are hashed together. They are stored in a multimap, mapping onto all rows containing identical groups of cell values. This provides a fast way to look up rows belonging to a set of cell values.


Probe Phase: HashJoinProbe
==========================

In the second phase, the HashJoinProbe PlanOperation is used. It accepts the second AbstractTable and the aforementioned HashTable as input to produce the resulting joined table:

.. code-block:: c
  :linenos:

	void HashJoinProbe::fetchPositions(                                                                                                                      
		pos_list_t *buildTablePosList,                                                                                                                       
		pos_list_t *probeTablePosList)                                                                                                                       
	{                                                                                                                                                        
		AbstractTable *probeTable = getProbeTable();                                                                                                         
		for (pos_t probeTableRow = 0; probeTableRow < probeTable->size(); ++probeTableRow)
		{
			pos_list_t *buildTableMatchingRows = getInputHashTable()->get(
				probeTable,
				_field_definition,
				probeTableRow);
			if (buildTableMatchingRows != NULL)
			{
				BOOST_FOREACH(pos_t buildTableRow, *buildTableMatchingRows)
				{
					buildTablePosList->push_back(buildTableRow);
					probeTablePosList->push_back(probeTableRow);
				}
			}
		}
	}

During execution, HashJoinProbe hashes the table's cell values of the specified row together on the fly while iterating over the table. For each calculated hash, a lookup on the HashTable is performed to fetch matching rows of the first table. Pairs of matching rows are stored in the resulting joined table.