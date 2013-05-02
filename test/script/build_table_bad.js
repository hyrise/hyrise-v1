function hyrise_run_op(input) {
	var result = buildTable();

	result.resize(4);
	result.setValueInt(0,0,1);
	result.setValueInt(0,1,2);
	result.setValueInt(0,2,3);
	result.setValueInt(0,3,4);

	result.setValueString(1,0,"Apple Inc");
	result.setValueString(1,1,"Microsoft");
	result.setValueString(1,2,"SAP AG");
	result.setValueString(1,3,"Oracle");

	return result;
}