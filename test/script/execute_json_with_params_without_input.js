function hyrise_run_op(input) {

    var jsonQuery ={ "operators": {
                        "load": {
                            "type": "TableLoad",
                             "table": "order_example",
                             "filename": "tables/companies.tbl"
                             },

                        "filter": {
                            "type": "SimpleTableScan",
                            "predicates" : [ { "type": "EQ", "in": 0, "f": "company_name", "value": "#{name1}", "vtype": 2} ]
                             }
                        },
                        
                    "edges" : [ ["load","filter"] ]
                    };


    var y = executeQuery(jsonQuery, null, {name1: "Apple Inc"});

    return y;
}