function hyrise_run_op(input) {
    jsonQuery ={ "operators": {
                        "load": {
                            "type": "TableLoad",
                            "table": "filterTest",
                            "filename": "tables/filter.tbl"
                        }
                    },
                    "edges" : [ ["load", "load"] ]
                };

    var filterTestTable = executeQuery(jsonQuery);

    var filtered = filterTestTable.filter(function(a, b) {
        return a == "Beter" || b >= 23;
    });

    return filtered;
}