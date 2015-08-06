function hyrise_run_op(input) {
    var filterTestTable = buildTableColumn({
        "NAME": "STRING",
        "AGE": "INTEGER",
        "HEIGHT": "INTEGER",
        "WEIGHT": "INTEGER"}, 5);

    var filtered = filterTestTable.filter(function(a, b) {
        return a == "Beter" || b >= 23;
    });

    return filtered;
}