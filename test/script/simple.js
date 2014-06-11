function hyrise_run_op() {
	var result = buildTable([{
		"type": "INTEGER",
		"name": "company_id"
	}, {
		"type": "STRING",
		"name": "company_name"
	}], [2]);

	return result;
}