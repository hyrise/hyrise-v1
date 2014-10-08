function hyrise_run_op(input) {

    var jsonQuery ={ "operators": {
                        "load": {
                            "type": "TableLoad",
                             "table": "order_example",
                             "filename": "tables/companies.tbl"
                             }
                        },
                        
                    "edges" : [ ["load","load"] ]
                    };


    var y = executeQuery(jsonQuery, null);

    return y;
}