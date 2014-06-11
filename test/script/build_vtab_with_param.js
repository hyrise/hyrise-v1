function hyrise_run_op(input, parameters) {
	var a = buildTable([
		{"type": "INTEGER", "name": "company_id"},
		],[1]);

	var b = buildTable([
		{"type": "STRING", "name": "company_name"}
		],[1]);

	a.resize(parameters["length"]);
	b.resize(parameters["length"]);

	for (i = 0; i < parameters["length"]; ++i) {
		a.setValueInt(0,i, i + 1);
		b.setValueString(0,i,parameters["company"]);
	}

	var result = buildVerticalTable(a, b);

	return result;
}