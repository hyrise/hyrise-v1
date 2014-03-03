###############
Parallelization
###############

HYRISE offers parallel execution of queries.

JSON is used to specify a human-readable definition of the query's operators. To execute them as specified, the QueryTransformationEngine transforms the query to a format which results in directly instantiable operator definitions using the QueryParser. The latter's resulting operators can finally be executed. See `Implementation Details`_.

The amount of parallel executable operators is limited by the thread pool size. There are two different kinds of parallel execution. Further details on how they work and rely on transformation can be found below.


Independent Operators
======================

Two operators are independent if they aren't nodes in the same edge. An edges definition like this allows operators "0" and "1" to be executed in parallel::

	"edges": [["0", "2"], ["1", "2"]]


Parallel Operator
==================

The above example is limited to execute operators as one. It is possible to execute a single operator in parallel using the "instances" JSON member in an operator's definition::

	{
		"operators": {
			"0": {
				[...]
				"instances": 2
				[...]
			},
			"1": { [...] }
		},
		"edges": [["0", "1"]]
	}

The QueryTransformationEngine recognizes "instances". The original operator "0" will be  replaced by its parallel instances. Additionally, the results of all instances will be combined using a UnionScan::

	{
		"operators": {
			"0_instance_0": {
				[...]
				"part": 0
				"count": 2
				[...]
			},
			"0_instance_1": {
				[...]
				"part": 1
				"count": 2
				[...]
			},
			"0_union": {
				"type": "UnionScan"
				"positions": true
			},
			"1": { [...] }
		},
		"edges": [
			["0_parallel_0", "0_union"],
			["0_parallel_1", "0_union"],
			["0_union", "1"]
		]
	}

As one can see, the edges will be adjusted accordingly. Consequently, "0"'s instances are independent of each other to allow parallel execution.

By default, the first input table is distributed about evenly on these instances using the "part" and "count" member variables and corresponding modulo distribution. Derived operators may overwrite _PlanOperation::splitInput for further behaviour.


Implementation Details
=======================

In this section, the QueryTransformationEngine and the QueryParser are explained in detail.


QueryTransformationEngine
--------------------------

As mentioned above, the QueryTransformationEngine transforms an incoming query. The resulting operator definitions and adapted edges have to be parseable one-to-one by the QueryParser to construct the correct corresponding executable task KV map. transform is the method to call::

	Json::Value &QueryTransformationEngine::transform(Json::Value &query) const
	{
		this->parallelizeOperators(query);
		return query;      
	}
	
Future transformations have to be placed here. Right now, parallelization is applied to operators::

	void QueryTransformationEngine::parallelizeOperators(Json::Value &query) const
	
Each operator with at least two instances gets parallelized::

	void QueryTransformationEngine::applyParallelizationTo(
		Json::Value &operatorConfiguration,
		const std::string &operatorId,
		Json::Value &query) const
	{
		std::string unionOperatorId = this->unionIdFor(operatorId);
		Json::Value unionOperator = this->unionOperator(operatorId);
		query["operators"][unionOperatorId] = unionOperator;
		const size_t numberOfInitialEdges = query["edges"].size();

		std::vector<std::string> *instanceIds = this->buildInstances(
			query, operatorConfiguration, operatorId, unionOperatorId);
		this->replaceOperatorWithInstances(
			operatorId, *instanceIds, unionOperatorId, query, numberOfInitialEdges);
 		delete instanceIds;
	}

As mentioned in `Parallel Operator`_, parallel instances' results are combined using a UnionScan. Instances are constructed and appended to the query's operator definitions using::

	std::vector<std::string> *QueryTransformationEngine::buildInstances(...) const

A vector of the instances' IDs is returned. The following method replaces the original operator in the query's operator definitions and edges with its instances using this vector::

	void QueryTransformationEngine::replaceOperatorWithInstances(...) const
	{
		this->appendInstancesDstNodeEdges(...);
		this->appendUnionSrcNodeEdges(...);
		this->removeOperatorNodes(...);
	}

Firstly, for each instance, edges with source nodes mapping to the original operator are duplicated to map to the instances.

Secondly, edges with the original operator as the source node areduplicated to have the parallelization's resulting UnionScan as the source node.

And lastly, the original operator's definition and all edges containing it in any node are removed. The operator's parallelization is completed.


QueryParser
------------

After transforming the incoming query to meet the specification, the QueryParser is used to instantiate each operator with correct edges-based dependencies. These so-called tasks are ready for execution. deserialize is the method to call::

	vector<boost::threadpool::dependency_task_func*> QueryParser::deserialize(
		Json::Value query,
		boost::threadpool::dependency_task_func **result) const
	{
		vector<boost::threadpool::dependency_task_func*> tasks;
		task_map_t task_map;

		this->buildTasks(query, tasks, task_map);
		this->setDependencies(query, task_map);
		*result = this->getResultTask(task_map);

		return tasks;
	}

Firstly, the tasks are build based on the query's operator definitions. Inside, Factories are used to call the parse method on the correct operator class (SimpleTableScan, JoinScan etc.) to get the operator object. This gets configured and appended to tasks. task_map is used to map its ID onto the object.

Secondly, dependencies between the tasks are set. The edges are simply traversed, looking up the operators' objects using their IDs and task_map.

And lastly, the result task returning the queries result is determined. It is defined as the one which has no successing task, independently of its position in the query's operator definitions or of its IDs. (Note: The ID is not allowed to be the autojson reference table ID.)

The vector of executable tasks is returned to the caller.
