Writing your own PlanOperation
******************************

This short tutorial demonstrates the basic steps toward implementing a Plan Operation in Hyrise, using a StringLength Operator as an example.

New Plan Operations inherit from PlanOperation.h and need to register themselves as Plan Operations using the following construct::

    bool NewPlanOperation::is_registered = 
        QueryParser::registerPlanOperation<NewPlanOperation>();

In addition to that new Plan Operations need to implement the following three methods (see below a simple example for a "StrLenScan" operation, that returns a list of string lengths for a given column of strings)::

1. Specifiy a name for your new Plan Operation using:
"""""""""""""""""""""""""""""""""""""""""""""""""""""
::

    static string name()
    {
        return "StrLenScan";
    }
    
as well as::

    const string vname() 
    {
        return "StrLenScan";
    }


2. Parse JSON input:
""""""""""""""""""""
::
    
    _PlanOperation *StrLenScan::parse(Json::Value &data)
    {
        _PlanOperation *p = BasicParser<StrLenScan>::parse(data);
        return p;
    }

In our case using a BasicParser Object (of type StrLenScan) is sufficient to parse all required input. BasicParser will parse JSON fields named "fields:" (representing columns the plan operation is to be executed on) and "limit:" (which we don't require in this example).

Additional JSON fields can be checked using ``data.isMember("someOtherField")``
and then accessed using ``data["someOtherField"][i]``


3. Implement your Plan Operation
""""""""""""""""""""""""""""""""
::

    void StrLenScan::executePlanOperation()
    {
	    const AbstractTable* in = input.getTable(0);

    	TableBuilder::param_list list;
    	list.append().set_type("INTEGER").set_name("str_len");
    	AbstractTable* string_length = TableBuilder::build(list);

    	if(in->typeOfColumn(_field_definition[0]) == StringType) {
    		for(size_t row = 0; row < in->size(); row++) {
    			string_length->setValue<int>(_field_definition[0], row, 
    			    (in->getValue<string>(_field_definition[0], row)).size());
    		}
    	}
    	
    	addResult(string_length);
    }

Implementation of your PlanOperation will vary with degree of complexity and might require you to write several helper methods. In this example, we create a new output table using TableBuilder (-> see TableBuilder.h). We then iterate over all the (string) elements of the input column and add the string's length as integer to the end of our output table (string_length).
Finally, to output the result of your plan operation, use ``this->addResult(output_table)``.


4. Testing your own Plan Operation
""""""""""""""""""""""""""""""""""

a simple query using the String Length Operator might look like this::

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

for our example list of students (located at test/students.csv) the output should look similar to this::

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
