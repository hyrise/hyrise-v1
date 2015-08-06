function hyrise_run_op(input) {
    jsonQuery ={ "operators": {
                        "filter": {
                            "type": "SimpleTableScan",
                            "predicates" : [
                                            { "type": "EQ", "in": 0, "f": "company_name", "value": "Apple Inc", "vtype": 2}
                                            ]
                             }
                        },
                    "edges" : [ ["filter","filter"] ]
                    };

    var result = executeQuery(jsonQuery, input);

    return result;
}