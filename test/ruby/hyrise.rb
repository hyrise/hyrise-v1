require 'rubygems'
require 'json'
require 'net/http'
require 'pp'

HYRISE_DEFAULT_URL = URI('http://localhost:5000/')

class HyriseConnection

  attr_reader :url

  def initialize(args = {})
    @args = { :url => HYRISE_DEFAULT_URL, :debug => false }.merge args
    if args[:url].class == String
      @url = URI(args[:url])
    elsif args[:url].class == URI
      @url = args[:url]
    end
  end
  
  def execute(query_or_operator)
    query_or_operator.execute @args
  end

end

class HyriseQuery
  attr_reader :operator_index

  def initialize(op = nil)
    @operator_index = 0
    @operators = []
    @operators_by_id = {}
    add_operator op if op
  end
  
  # Adds an operator or an array of operators to this query
  def add_operator(operator)
    if operator.class == Array
      operator.each { |op| add_operator op }
    else
      return if @operators.include? operator
      operator.index = @operator_index.to_s
      @operator_index += 1
      @operators << operator      
      @operators_by_id[operator.index] = operator
      add_operator operator.dependencies
    end
  end
  
  # Synonym for add_operator
  def <<(other)
    add_operator other
  end
  
  # Executes this query and returns the server response
  def execute(args = {})
    args = { :url => HYRISE_DEFAULT_URL, :debug => false }.merge args    
    if args[:url].instance_of? String
      url = URI(args[:url])
    else
      url = args[:url]
    end
    
    query = {'operators' => {}, 'edges' => []}
    @operators.each { |op| 
      query['operators'][op.index] = { "type" => op.name }.merge(op.attributes)
      op.dependencies.each { |dep|
        query['edges'] << [dep.index, op.index]
      }
    }
    
    request_body = query.to_json

    req = Net::HTTP::Post.new(url.path)
    req.set_form_data({:query=> request_body})

    response = Net::HTTP.new(url.host, url.port).start {|http|
      http.read_timeout = nil
      http.request(req)
    }
    
    response_body = response.body
    json = JSON.parse(response_body)
    json["performanceData"].each { |pd|
      if @operators_by_id.has_key? pd["id"]
        @operators_by_id[pd["id"]].performanceData = pd
      else
        op = HyriseOperator.new pd["name"]
        op.performanceData = pd
        op.index = pd["id"]
        @operators_by_id[op.index] = op
        @operators << op
      end
    }
    
    if args[:debug]
      File.open("query_gantt.svg", "w") do |svgfile|
        svgfile.write(gantt)
      end
      File.open("query_plan.dot", "w") do |svgfile|
        svgfile.write(dot)
      end
      File.open("query_response.json", "w") do |svgfile|
        svgfile.write(response_body)
      end
    end
    
    return json
  end
  
  def gantt
    require 'builder'
  
    buffer = ""
    svg_width = 600
    xml = Builder::XmlMarkup.new(:target => buffer, :indent => 1)
    xml.instruct!
    xml.svg :xmlns => "http://www.w3.org/2000/svg", :version => "1.1" do |svg|
      endtime = @operators.collect { |op| op.performanceData["endTime"].to_f }.max * 1.1
      threads = @operators.collect { |op| op.performanceData["executingThread"] }.uniq
      
      thread_index = 0
      threads.each { |thread|
        svg.line :x1 => 0, :x2 => svg_width, :y1 => thread_index * 50 + 25, :y2 => thread_index * 50 + 25, :style => "stroke-width:1; stroke:rgb(127,127,127);"
        svg.text "Thread #{thread}", :x => 5, :y => thread_index * 50 + 15, :style => "font-size: 8pt; fill:rgb(127,127,127)"
        thread_index += 1
      }

      @operators.each { |operator|
        next if operator.name == "InputSelectorOperation"
        threadIndex = threads.index operator.performanceData["executingThread"]
        start = operator.performanceData["startTime"].to_f
        duration = (operator.performanceData["endTime"] - operator.performanceData["startTime"]).to_f
        
        x = start / endtime * svg_width
        width = duration / endtime * svg_width

        y = threadIndex * 50
        svg.rect :x => x, :y => y, :width => width, :height => 40, :style => "fill:rgb(200,200,200); stroke-width:1; stroke:rgb(0,0,0);"    
        
        svg.text "#{operator.name} (#{duration} ms)", :x => x + 2, :y => y + 16, :style => "font-size: 8pt", :transform => "rotate(90 #{x + 2} #{y + 16})"
      }
    end
    buffer
  end
  
  # Returns a dot representation of this query
  def dot
    s = "digraph query {\n"
    @operators.each { |op| 
      desc = op.attributes.inspect.gsub("\"", "\\\"")
      label = "[#{op.index}] #{op.name}"
      if op.performanceData
        duration = (op.performanceData["endTime"] - op.performanceData["startTime"]).to_f
        label += " (#{duration} ms)"
      end
      s += "  #{op.index} [label=\"#{label}\n #{desc}\", shape=box];\n"
      op.dependencies.each { |dep|
        s += "  #{dep.index} -> #{op.index};\n"
      }
    }    
    s += "}\n"
  end
  
end

class HyriseOperator
  attr_accessor :index
  attr_accessor :performanceData
  attr_reader :dependencies
  attr_reader :attributes
  attr_reader :name

  def initialize(name, attributes = {})
    @name = name.to_s
    @dependencies = []
    @attributes = attributes
  end
  
  def add_dependency(other)
    @dependencies << other
  end
  
  def add_dependencies(others)
    @dependencies += others
  end
  
  def execute(args = {})
    query = HyriseQuery.new self
    query.execute args
  end

  def dot
    query = HyriseQuery.new self
    query.dot
  end
  
end

class HyriseExpression

  attr_accessor :operator
  attr_accessor :operands

  def self.or_exp(o1, o2)
    self.new({ :type => "OR" }, [o1, o2])
  end

  def self.and_exp(o1, o2)
    self.new({ :type => "AND" }, [o1, o2])
  end
  
  def self.not_exp(o1)
    self.new({ :type => "NOT" }, [o1])
  end

  def self.equals_exp(field, value)
    vtype = case value
    when Fixnum
      0
    when Float
      1
    when String
      2
    end
    self.new({ :type => "EQ", :f => field, :in => 0, :value => value, :vtype => vtype })
  end
  
  def initialize(operator, operands = [])
    @operator = operator
    @operands = operands
  end

  def to_predicates
    [@operator] + @operands.inject([]) { |list,op| list += op.to_predicates }
  end

end

class HyriseScanOperator < HyriseOperator

  def initialize(expression)
    super(:SimpleTableScan, { :predicates => expression.to_predicates })
  end

end
