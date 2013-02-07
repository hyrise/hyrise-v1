require 'hyrise'

begin
  require "test/#{ARGV[0].to_s}"
rescue LoadError
  require "queries/#{ARGV[0].to_s}"
end
plan.execute(:debug=>true)
