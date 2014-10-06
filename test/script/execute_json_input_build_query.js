function hyrise_run_op(input) {
    var operators = {
                    "filter": {
                        "type": "SimpleTableScan",
                        "predicates" : [
                            { "type": "EQ", "in": 0, "f": "company_name", "value": "Apple Inc", "vtype": 2}
                        ]
                    }
                }
    var edges = [ ["filter","filter"] ];

    var jsonQuery = buildQuery(operators, edges);

    var result = executeQuery(jsonQuery, input);

    return result;
}