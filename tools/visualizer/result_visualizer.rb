require 'rubygems'
require 'builder'
require 'json'
require 'optparse'

def  build(operators)
	buffer = ""
    svg_width = 3000

    # We have to normalize to a minium to that we make sure, that the smallest element has size 50
    endtime = operators.collect { |op| op["endTime"].to_f }.max * 1
    threads = operators.collect { |op| op["executingThread"] }.uniq

    # Calculate size factor
    min_width = operators.collect { |operator|
    	duration = (operator["endTime"] - operator["startTime"]).to_f
        duration / endtime * svg_width
    }.min

    # Calculate the color setting
    durations = operators.collect { |operator|
    	(operator["endTime"] - operator["startTime"]).to_f
    }

    min_dur = durations.min
    max_dur = durations.max
    dur_factor = (max_dur - min_dur) / 255

    size_factor = 0.1/min_width
    thread_width = 200
    start_offset = 30    

    svg_width *= size_factor

    xml = Builder::XmlMarkup.new(:target => buffer, :indent => 1)
    xml.instruct!
    xml.svg :xmlns => "http://www.w3.org/2000/svg", :version => "1.1", :height => svg_width, :width => threads.size * thread_width do |svg|
      
      
      thread_index = 0
      threads.each { |thread|
        svg.line :y1 => start_offset, :y2 => svg_width, :x1 => thread_index * thread_width + 25, :x2 => thread_index * thread_width + 25, :style => "stroke-width:1; stroke:rgb(127,127,127);"
        svg.text "Thread #{thread}", :y => 15, :x => thread_index * thread_width + 15, :style => "font-size: 10pt; fill:rgb(127,127,127)"
        thread_index += 1
      }

      operators.each { |operator|
        next if operator["name"] == "InputSelectorOperation"
        threadIndex = threads.index operator["executingThread"]
        start = operator["startTime"].to_f
        duration = (operator["endTime"] - operator["startTime"]).to_f
        
        x = start / endtime * svg_width + start_offset
        width = duration / endtime * svg_width
        y = threadIndex * thread_width

        duration_color = ((duration - min_dur) / dur_factor).round


        svg.rect :y => x, :x => y, :height => width, :width => 40, :style => "fill:rgb(#{duration_color},89,0); stroke-width:1; stroke:rgb(0,0,0);"    
        
        svg.text "#{operator["name"]} (#{duration.round} ms)", :y => x + 10, :x => y + 50, :style => "font-size: 8pt", :transform => "rotate(0 #{y + 10} #{x + 2})"
      }
    end
    buffer
end

def run(input, options)
	data = JSON.parse(File.read(input))
	buffer = build(data["performanceData"])

	output = File.basename(input) << ".svg"
	unless options[:use_curr_dir]
		output = options[:output_file]
	end

	File.open(output, "w+") do |f|
		f.write(buffer)
	end
end


options = {}

optparse = OptionParser.new do|opts|
	opts.banner = <<-EOF 
HYRISE result visualizer. The input is the JSON file the output
will be an EPS with the visualized result.

Usage: result_visualizer [-o] plan.json
EOF
	options[:output_file] = nil
	options[:use_curr_dir] = true
	opts.on('-o', '--output-file OUTPUT', 'Set the output file for the SVG') do |f|
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