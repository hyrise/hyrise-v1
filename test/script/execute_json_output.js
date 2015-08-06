function hyrise_run_op(input) {
    var jsonQuery ={    "papi": "PAPI_TOT_INS",
                     "operators": {
                         "load_example": {
                             "type": "TableLoad",
                             "table": "order_example",
                             "filename": "tables/companies.tbl"
                             }
                         },
                     "edges" : [["load_example","load_example"]]
                 }

    var result = executeQuery(jsonQuery);

    return result;
}