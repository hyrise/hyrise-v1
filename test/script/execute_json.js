function hyrise_run_op(input) {


    var result = buildTable([
        {"type": "INTEGER", "name": "company_id"},
        {"type": "STRING", "name": "company_name"}
        ],[2]);

    result.resize(4);
    result.setValueInt(0,0,1);
    result.setValueInt(0,1,2);
    result.setValueInt(0,2,3);
    result.setValueInt(0,3,4);

    result.setValueString(1,0,"Apple Inc");
    result.setValueString(1,1,"Microsoft");
    result.setValueString(1,2,"SAP AG");
    result.setValueString(1,3,"Oracle");

    // var jsonQuery ='{    "papi": "PAPI_TOT_INS",\
    //                  "operators": {\
    //                      "load_example": {\
    //                          "type": "TableLoad",\
    //                          "table": "order_example",\
    //                          "filename": "small.tbl"\
    //                          }\
    //                      },\
    //                  "edges" : [["load_example","load_example"]]\
    //              }'
    
    var jsonQuery ='{ "operators": {\
                        "filter": {\
                            "type": "SimpleTableScan",\
                            "predicates" : [ \
                                            {"type": "OR"},\
                                            { "type": "EQ", "in": 0, "f": "company_name", "value": "Apple Inc", "vtype": 2}, \
                                            { "type": "EQ", "in": 0, "f": "company_name", "value": "Microsoft", "vtype": 2}  \
                                            ]\
                             }\
                        },\
                    "edges" : [ ["filter","filter"] ]\
                    }'

    var x = executeQuery(jsonQuery, result);

    jsonQuery ='{ "operators": {\
                        "filter": {\
                            "type": "SimpleTableScan",\
                            "predicates" : [ \
                                            { "type": "EQ", "in": 0, "f": "company_name", "value": "Apple Inc", "vtype": 2}\
                                            ]\
                             }\
                        },\
                    "edges" : [ ["filter","filter"] ]\
                    }'

    var y = executeQuery(jsonQuery, x);


    return y;
}