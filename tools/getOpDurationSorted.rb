require "json"

def main(file)
  perfData = JSON.load(File.new(file).read)["performanceData"]
  a = perfData.
    sort{|a,b| a["startTime"] <=> b["startTime"]}.
    collect{|op| [op["id"], op["endTime"] - op["startTime"]] }
  puts a
end

def usage()
  puts "Returns the duration of all operations in a json response of Hyrise."
  puts "Results are sorted in decreasing order of the duration."
  puts ""
  puts "Usage: ruby getOpDurationSorted.rb <HYRISE_RESPONSE.json>"
end

if (ARGV.length != 1) then
  usage()
else
  main(ARGV[0])
end

