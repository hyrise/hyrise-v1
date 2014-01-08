// Initial data
window.result = {
	rows : [],
	header : [],
	per_page : 20,
	page_count : 0
};


function simpleFormat(number, traling) {
	return Math.round(number * Math.pow(10, traling)) / Math.pow(10, traling);
}

function parsePerformanceData(data) {
	var result = {};
	$.each(data, function(i, v){
		result[v["id"]] = v["endTime"] - v["startTime"];
	});
	return result;
}

function totalTime(data) {
	var result = 0.0;
	$.each(data, function(i, v){
		console.log(v["id"]);
		if (v["id"] == "respond")
			result = v["endTime"];
	});
	return simpleFormat(result, 2);
}

function addQuery(q) {
    if (localStorage) {
	if (q["id"]) {
	    localStorage[q["id"]] = JSON.stringify(q, null, 2);
	} else {
	    var d = new Date();
	    localStorage[d.toISOString()] = JSON.stringify(q, null, 2);
	}
    }
}

function getQuery(id) {
    if (localStorage) {
	return localStorage[id];
    }
}

function getQueries() {
    var result = {};
    for (var i=0; i < localStorage.length; ++i) {
	var k = localStorage.key(i);
	result[k] = localStorage[k];
    }
    return result;
}

function updateQueryHistory() {
    $("#query_history").html("");
    $("#query_history").append("<option></option>");
    // Get all history queries
    var queries = getQueries();
    $.each(queries, function(k,v) {
      $("#query_history").append("<option value=\"" + k + "\">"+ k + "</option>");
    });

}

function runQuery() {
	// Clear the inner HTML
	$("#msg").empty();
	$("#msg_error").empty();
	$("#json_result").empty();
	//$("#btn_submit").button("loading");
	$("#txtquery").attr("rows", 2);

	

	$.ajax({
		type: "POST",
		url: "/query",
		data: $("#query_form").serialize(),
		cache: false,
		success: function(d, status){

			$("#btn_submit").button("reset");

			// Check for error messages
			if (d["error"]) {
				$("#msg_error").html(d["error"]);
				return;
			} else {
				$("#msg").html("Query Executed.");
			}

			// Do the dot transformation
                        if (typeof(d) != "object")
			  d = JSON.parse(d);

                        var performance = null;
                        if (d["performanceData"]) {
			  performance = parsePerformanceData(d["performanceData"]);
			  $("#msg").append(" " + totalTime(d["performanceData"]) + "ms");
                        }

		        // Add Query to localstorage
		        var qJson = JSON.parse($("#txtquery").val());
                      
			var svg = Viz(toDot(qJson, performance), "svg");
			if (svg) {
			  $("#query_plan").html(svg);
			}

			$("#result_view").show();

			if (!d["header"]) {
				$("#btn_plan").click();
				return;
			}

			// Reset the result data
			window.result.data = []
			window.result.header = []

			$("#btn_result").click();

			window.result.header = d["header"];
			window.result.rows = d["rows"];

			// Init pagination
			var total_pages = window.result.rows.length / window.result.per_page;
			total_pages = total_pages < 1 ? 1 : total_pages;

			$('#page-selection').bootpag({
				page: 1,
				maxVisible: 10,
				total: total_pages
			}).on("page", printQueryResult);
			printQueryResult(null, 0);
		},
		error: function(e) { console.log(e);}
	})

	// Prevent default
	return false;
}

function printQueryResult(event, page_num) {
	var element = $("#json_result");

	// Header first
	var result = "<table class=\"hyrise_tab table table-striped table-bordered\">";
	result += "<tr>";
	for(var i=0; i < window.result.header.length; ++i) {
		result += "<th>" + window.result.header[i] + "</th>";
	}
	result += "</tr>";

	// Now the rows
	var start = window.result.per_page * page_num;
	var stop = window.result.per_page * (page_num + 1);

	if (stop > window.result.rows.length)
		stop = window.result.rows.length;

	for (var i = start; i < stop; i++) {
		var row = window.result.rows[i];
		result += "<tr>";
		for (var j = 0; j < row.length; j++) {
			var item = row[j];
			result += "<td>" + item + "</td>";
		};
		result += "</tr>";
	};
	result += "</table>";	
	element.html(result);
}


/////////////////////////////////////////////////////////////////////////////////
// Plan Visualizer
function makeKey(key) {
	if (typeof(key) == "number") {
		if (key > 0)
			return key + "";
		else
			return (key+"").replace("-", "___");
	}
	return key + "";
}


function maxPerf(performance) {
	var result  = 0;
        if (performance)
	  $.each(performance, function(k,v){result = v > result ? v : result;});
	return result;
}

function minPerf(performance) {
	var result = Number.MAX_VALUE;
        if (performance)
	  $.each(performance, function(k,v){result = v < result ? v : result;});
	return result;
}

function toDot(json_graph, performance) {

	// Scaling of the nodes
	var minp = minPerf(performance);
	var maxp = maxPerf(performance);
	var minf = 8;
	var maxf = 30;
	var stepping = 1;
	var numSteps = (maxf - minf) / stepping;
	var perfStep = (maxp - minp) / numSteps;


	var result = "digraph query {\n";
	result += "node[shape=\"record\"];\n";

	$.each(json_graph["operators"], function(key, value){
		result += "operator_" + makeKey(key) + " [label=\"{"+value["type"]+"|";

                if (performance)
		  result += simpleFormat(performance[key], 4) + " ms";

		result +="}\""; 
          
                if (performance) {
		  var fontSize = Math.round((performance[key] - minp) / perfStep)  * stepping + minf;
		  result += ",fontsize="+ fontSize;
                }

		result += "];\n"
	});
	$.each(json_graph["edges"], function(i,v){
		result += "operator_" + makeKey(v[0]) + " -> operator_" + makeKey(v[1]) + ";\n"
	});
	result += "}\n";
	console.log(result);
	return result;
}
