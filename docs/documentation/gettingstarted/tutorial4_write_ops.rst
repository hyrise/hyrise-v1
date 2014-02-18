##############################
Writing your own PlanOperation
##############################

This short tutorial demonstrates the basic steps toward implementing a Plan Operation in Hyrise, using a StringLength Operator as an example.

New Plan Operations inherit from PlanOperation.h and need to register themselves as Plan Operations using the following construct::

    auto _ =  QueryParser::registerPlanOperation<NewPlanOperation>("NameOfNewPlanOperation");

In addition to that new Plan Operations need to implement the following two methods (see below a simple example for a "StrLenScan" operation, that returns a list of string lengths for a given column of strings)::


1. Parse JSON input:
====================

::
    
    std::shared_ptr<PlanOperation> StrLenScan::parse(const Json::Value &data)
    {
        std::shared_ptr<StrLenScan> instance = BasicParser<StrLenScan>::parse(data);
        return instance;
    }

In our case using a BasicParser Object (of type StrLenScan) is sufficient to parse all required input. BasicParser will parse JSON fields named "fields:" (representing columns the plan operation is to be executed on) and "limit:" (which we don't require in this example).

Additional JSON fields can be checked using ``data.isMember("someOtherField")``
and then accessed using ``data["someOtherField"][i]``.

If you only use the basic parser, instead of adding this method, you can also omit it and register your operator with ``auto _ =  QueryParser::registerTrivialPlanOperation<NewPlanOperation>("NameOfNewPlanOperation")``. This will automatically register the operator and tells Hyrise to use the basic JSON parser for it.


2. Implement your Plan Operation
================================

::

    void StrLenScan::executePlanOperation()
    {
	    std::shared_ptr<const storage::AbstractTable> in = input.getTable(0);

    	TableBuilder::param_list list;
    	list.append().set_type("INTEGER").set_name("LENGTH");
    	std::shared_ptr<storage::AbstractTable> resultTable = storage::TableBuilder::build(list);

        resultTable->resize(in->size());

    	if(in->typeOfColumn(_field_definition[0]) == StringType) {
    		for(size_t row = 0; row < in->size(); row++) {
    			resultTable->setValue<hyrise_int_t>(0, row, 
    			    (in->getValue<std::string>(_field_definition[0], row)).length());
    		}
    	} else {
            throw std::runtime_error("column not of type string.");
        }
    	
    	addResult(resultTable);
    }

The implementation of your PlanOperation will vary with degree of complexity and might require you to write several helper methods. In this example, we create a new output table using TableBuilder (-> see TableBuilder.h). We then iterate over all the (string) elements of the input column and add the string's length as integer to the end of our output table (string_length).
Finally, to output the result of your plan operation, use ``this->addResult(output_table)``.


3. Testing your own Plan Operation
==================================

To test your operator just add a query using it to the test/autojson directory. A simple query using the String Length Operator might look like this::

    {
    	"operators": {
    		"0": {
    			"type": "MySQLTableLoad",
    			"table": "students",
    			"database": "epic"
    		},
    		"1": {
    		    "type": "StrLenScan",
    		    "fields": ["name"]
    		}
    	},
    	"edges": [["0", "1"]]
    }

For our example list of students (located at test/students.csv) the output should look similar to this::

    {
	   "header" : [ "str_len" ],
	   "performanceData" : 
	   [
            {
                "data" : 0,
                "duration" : 15743,
                "endTime" : 1503.7235250,
                "executingThread" : "0x7fc64b420c10",
                "id" : "0",
                "name" : "MySQLTableLoad",
                "papi_event" : "PAPI_TOT_INS",
                "startTime" : 65.3545410
            },
            {
                "data" : 0,
                "duration" : 42,
                "endTime" : 1503.9631340,
                "executingThread" : "0x7fc64b420c10",
                "id" : "1",
                "name" : "StrLenScan",
                "papi_event" : "PAPI_TOT_INS",
                "startTime" : 1503.8894410
            },

            {
                "endTime" : 65.2453010,
                "executingThread" : "0x7fc64b4225a0",
                "id" : "requestParse",
                "name" : "RequestParseTask",
                "startTime" : 0.0
            },

            {
                "duration" : 158,
                "endTime" : 1504.1458390,
                "executingThread" : "0x7fc64b420a00",
                "id" : "respond",
                "name" : "ResponseTask",
                "startTime" : 1503.9848240
            }
        ],
        "rows" : 
              [
        	[ 13 ], ..., [ 19 ]
	]
    }

For a complete test you should also specify an output table for the query result to be compared with. To do this you add a table load operation to the beginning your test query having the index '1' and a table field with the value 'reference'::

    "operators": {
        "-1" : {
          "type" : "TableLoad",
          "filename" : "tables/revenue.tbl",
          "table" : "reference"
        },
        ...


4. Full example
===============

StrLenScan.cpp:

.. code-block:: cpp
    :linenos:

    #include "access/StrLenScan.h"

    #include "access/system/BasicParser.h"
    #include "access/system/QueryParser.h"

    #include "storage/AbstractTable.h"
    #include "storage/TableBuilder.h"

    namespace hyrise {
    namespace access {

    auto _ = QueryParser::registerPlanOperation<StrLenScan>("StrLenScan");

    StrLenScan::~StrLenScan() {
    }

    void StrLenScan::executePlanOperation() {
      
      std::shared_ptr<const storage::AbstractTable> in = input.getTable(0);

      storage::TableBuilder::param_list list;
      list.append().set_type("INTEGER").set_name("LENGTH");
      std::shared_ptr<storage::AbstractTable> resultTable = storage::TableBuilder::build(list);

      resultTable->resize(in->size());

      for(size_t row = 0; row < in->size(); row++) {
        resultTable->setValue<hyrise_int_t>(0, row, 
          (in->getValue<std::string>(_field_definition[0], row)).length() );
      }

      addResult(resultTable);
    }

    std::shared_ptr<PlanOperation> StrLenScan::parse(const Json::Value &data) {
      std::shared_ptr<StrLenScan> instance = BasicParser<StrLenScan>::parse(data);

      return instance;
    }

    const std::string StrLenScan::vname() {
      return "StrLenScan";
    }

    }
    }


StrLenScan.h:

.. code-block:: cpp
    :linenos:
    
    #ifndef SRC_LIB_ACCESS_STRLENSCAN_H_
    #define SRC_LIB_ACCESS_STRLENSCAN_H_

    #include "access/system/PlanOperation.h"

    namespace hyrise {
    namespace access {

    class StrLenScan : public PlanOperation {
    public:
      virtual ~StrLenScan();

      void executePlanOperation();
      static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
      const std::string vname();
    };

    }
    }
    #endif  // SRC_LIB_ACCESS_STRLENSCAN_H_
