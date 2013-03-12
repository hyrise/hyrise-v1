function runQuery() {
	// Clear the inner HTML
	$("#json_result").empty();
	$("#btn_submit").button("loading");
	
	$.ajax({
		type: "POST",
		url: "/query",
		data: $("#query_form").serialize(),
		cache: false,
		success: function(d, status){
			printQueryResult(d, $("#json_result"));
		},
		error: function(e) { console.log(e);}
	})

	// Prevent default
	return false;
}

function printQueryResult(data, element) {
	$("#btn_submit").button("reset");
	$("#msg").html("Query Executed.");


	if (!data["header"])
		return;

	// Header first
	var result = "<table class=\"hyrise_tab table table-striped table-bordered\">";
	result += "<tr>";
	for(var i=0; i < data["header"].length; ++i) {
		result += "<th>" + data["header"][i] + "</th>";
	}
	result += "</tr>";

	// Now the rows
	for (var i = 0; i < data["rows"].length; i++) {
		var row = data["rows"][i];
		result += "<tr>";
		for (var j = 0; j < row.length; j++) {
			var item = row[j];
			result += "<td>" + item + "</td>";
		};
		result += "</tr>";
	};
	result += "</table>";	
	element.append(result);
}