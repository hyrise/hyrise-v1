function hyrise_run_op(input) {
	var a = buildTable([
		{"type": "INTEGER", "name": "company_id"},
		],[1]);

	var b = buildTable([
		{"type": "STRING", "name": "company_name"}
		],[1]);

	a.resize(4);
	a.setValueInt(0,0,1);
	a.setValueInt(0,1,2);
	a.setValueInt(0,2,3);
	a.setValueInt(0,3,4);

	b.resize(4);
	b.setValueString(0,0,"Apple Inc");
	b.setValueString(0,1,"Microsoft");
	b.setValueString(0,2,"SAP AG");
	b.setValueString(0,3,"Oracle");

	var result = buildVerticalTable(a, b);

	return result;
}