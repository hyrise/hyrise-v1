#############
PlanOperation
#############


Input/output handling
=====================

PlanOperations accept multiple data types as input and output data. Though most operations perform on AbstractTable so far, the HashJoinProbe for example expects a HashTable as an input. The HashBuild operator produces HashTables. Therefore, all PlanOperations store their input and output data in OperationalData containers::

	class _PlanOperation : public OutputTask
	{
	protected:
		OperationalData input;
		OperationalData output;
	...

These simply hold lists of data::

	class OperationalData
	{
	protected:
		vector<AbstractTable *> tables;
		vector<HashTable *>     hashTables;
	...

To implement more data types, add additional lists and implement their corresponding interfaces. (get, add etc.) Interfaces have to be implemented on the PlanOperation as well, e.g. addInput, getInput(size_t index) or addResult etc.

During query execution, these containers are passed between dependent operators. On execution, PlanOperation::refreshInput is called to fetch output data of predecessors. Override setupPlanOperation to implement additional processing behaviour.

Always keep an eye on retain counting correctness to prevent memory leaks or to prevent fetching deleted objects!

For implementation details of two example operators go to :doc:`./groupby` and :doc:`./hashjoin`.
