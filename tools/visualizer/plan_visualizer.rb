require 'rubygems'
require 'json'
require 'tempfile'
require 'optparse'

FIELD_PROC = lambda { |object| 
	"fields: [#{object["fields"].join(",")}]"
}

EXPRESSION_TYPES = {
	0 => "=", 
	1 => "\\<",
	2 => "\\>",
	3 => "between",
	5 => "not",
	6 => "AND",
	7 => "OR",
	8 => "NOT"
}

JOIN_EXPRESSION_TYPES = {
	0 => "AND",
	1 => "OR",
	2 => "NOT",
	3 => "EQ"
}

OPERATOR_MAP = {
	"SimpleTableScan" => lambda { |object| 
		result = "predicates: [" << object["predicates"].collect do |p|
			result = "#{p["f"]} #{EXPRESSION_TYPES[p["type"]]} #{p["value"].to_s.gsub(":","")}"
		end.join(",") << "]\\l"

		if object.key?("materializing")
			result << "materializing: #{object["materializing"].to_s}"
		else
			result << "materializing: false"
		end
		result
		},
	"JoinScan" => lambda { |object| 
		"predicates: [" << object["predicates"].collect do |p|
			if p["type"].class == String
				result = "(#{p["input_left"]}:#{p["field_left"]} #{p["type"]} #{p["input_right"]}:#{p["field_right"]})"
			elsif p["type"] >= 3
				result = "(#{p["input_left"]}:#{p["field_left"]} #{JOIN_EXPRESSION_TYPES[p["type"]]} #{p["input_right"]}:#{p["field_right"]})"
			else
				result = "#{JOIN_EXPRESSION_TYPES[p["type"]]}"
			end
		end.join(" ") << "]"
		},	
	"TableLoad" => lambda { |object| 
		"table: #{object["table"]}\\lfilename:#{object["filename"]}"
	 },
	 "SortScan" => FIELD_PROC,
	 "ProjectionScan" => FIELD_PROC,
	 "HashBuild" => FIELD_PROC,
	 "HashJoinProbe" => FIELD_PROC,
	 "GroupByScan" => lambda{ |object|
	 	result = "" << FIELD_PROC.call(object) << "\\l"

	 	result << "functions: ["
	 	result << object["functions"].collect do |p|
	 		if p["type"] == 0
	 			"SUM(#{p["field"]})"
	 		elsif p["type"] == 1
	 			"COUNT(#{p["field"]})"
	 		elsif p["type"] == 2
	 			"AVG(#{p["field"]})"
	 		end
	 	end.join(",")
	 	result << "]"
	 },
	 "MySQLTableLoad" => lambda do |object|
	 	"table: #{object["table"]}\\ldatabase: #{object["database"]}"
	 end
}



def make_key(key)
	return key if key.to_i > 0
	nkey = key.to_i < 0 ? key.to_s.gsub("-", "___") : key.to_s
	nkey
end	


def to_dot(json_graph)

	output = "digraph query {\n"

	output << "node[shape=\"record\"];\n"

	json_graph["operators"].each do |pair|

		output << "operator_#{make_key(pair[0])} [label=\"{#{pair[1]["type"]}|"

		if OPERATOR_MAP.key?(pair[1]["type"])
			output << OPERATOR_MAP[pair[1]["type"]].call(pair[1])
		end	

		output << "}\"];\n"
	end	

	json_graph["edges"].each do |pair|
		output << "operator_#{make_key(pair[0])} -> operator_#{pair[1]};\n"
	end

	output << "}\n"
end


def load(json_graph)
	JSON.parse(File.read(json_graph))
end	

def call_dot(data, ofile)
	Tempfile.open(['plan_visualizer', "json"])  do |f|
		f.write(data)
		f.flush
		cmd = %x[dot -Teps -o #{ofile} #{f.path}]
	end
end

def run(input, options)
	plan_file = input
	json_plan = load(plan_file)
	dot_plan = to_dot(json_plan)

	output = File.basename(plan_file) << ".eps"
	unless options[:use_curr_dir]
		output = options[:output_file]
	end

	call_dot(dot_plan, output)
end


options = {}

optparse = OptionParser.new do|opts|
	opts.banner = <<-EOF 
HYRISE plan visualizer. The input is the JSON file the output
will be an EPS with the visualized plan.

Usage: plan_visualizer [-o] plan.json
EOF
	options[:output_file] = nil
	options[:use_curr_dir] = true
	opts.on('-o', '--output-file OUTPUT', 'Set the output file for the EPS') do |f|
		options[:output_file] = f
		options[:use_curr_dir] = false
	end	
	opts.on("-h", "--help", "Show this help message.") { puts opts; exit }
end

optparse.parse!

if ARGV.size != 1
	puts optparse
	exit
end

run(ARGV[0], options)