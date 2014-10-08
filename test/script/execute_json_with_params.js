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

    var jsonQuery ={ "operators": {
                        "filter": {
                            "type": "SimpleTableScan",
                            "predicates" : [
                                        {"type": "OR"},
                                        {"type": "OR"},
                                            { "type": "EQ", "in": 0, "f": "company_id", "value": "#{id}", "vtype": 0},
                                            { "type": "EQ", "in": 0, "f": "company_name", "value": "#{name1}", "vtype": 2},
                                            { "type": "EQ", "in": 0, "f": "company_name", "value": "#{name2}", "vtype": 2}
                                            ]
                             }
                        },
                    "edges" : [ ["filter","filter"] ]
                    };


    var y = executeQuery(jsonQuery, result, {id: 1, name1: "Microsoft", name2: "SAP AG"});

    return y;
}