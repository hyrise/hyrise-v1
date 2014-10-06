function hyrise_run_op(input) {


    var result = buildTable([
        {"type": "INTEGER", "name": "company_id"},
        {"type": "FLOAT", "name": "company_revenue"}
        ],[2]);

    result.resize(4);
    result.setValueInt(0,0,0);
    result.setValueInt(0,1,1);
    result.setValueInt(0,2,2);
    result.setValueInt(0,3,3);

    result.setValueFloat(1,0,2500.0);
    result.setValueFloat(1,1,7800.3);
    result.setValueFloat(1,2,12400.7);
    result.setValueFloat(1,3,9514.6);

    var jsonQuery ={ "operators": {
                        "filter": {
                            "type": "SimpleTableScan",
                            "predicates" : [
                                        {"type": "OR"},
                                        {"type": "OR"},
                                            { "type": "EQ", "in": 0, "f": "company_id", "value": "#{id1}", "vtype": 0},
                                            { "type": "EQ", "in": 0, "f": "company_id", "value": "#{id2}", "vtype": 0},
                                            { "type": "EQ", "in": 0, "f": "company_revenue", "value": "#{revenue}", "vtype": 1}
                                            ]
                             }
                        },
                    "edges" : [ ["filter","filter"] ]
                    };

    // Yes, mapping boolean on an INTEGER column is an ugly hack, but works for testing the parser
    var y = executeQuery(jsonQuery, result, {id1: false, id2: true, revenue: 9514.6});

    return y;
}