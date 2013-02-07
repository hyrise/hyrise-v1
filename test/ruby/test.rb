
require 'rubygems'
gem 'minitest'
gem 'json'

require 'minitest/autorun'

require 'extensions'
require 'hyrise'

RUN_HYRISE_SERVER = true

class HyriseTest <  HyriseTestCase

  def self.prepare
    files = Dir.glob(File.join(Dir.pwd, File.dirname(__FILE__), 'test', '*.rb')).sort
    files.each do |rubyfile|
      base = rubyfile[0..-4]
      jsonpath = base + ".json"

      send :define_method, "test_auto_#{File.basename(base)}" do
        require base
        execute_compare_op(plan, jsonpath)
      end
    end
  end

  prepare
  
  def self.before_suite
    HyriseHooks.startup
  end

  def self.after_suite
    HyriseHooks.shutdown
  end
  
  def execute_compare_op(operator, expected_filename)
    result = operator.execute["rows"]
    if FileTest.exist? expected_filename
      expected = open(expected_filename, 'r') do |f|
        JSON.parse(f.read())
      end
      assert_equal(result, expected, "Result should equal saved result")
    else
      assert(result, "Expected result")

      if ENV["HRYSIE_RUBY_CREATE_JSON"]
        puts "  #{File.basename(expected_filename)} is missing, creating it"
        expected = open(expected_filename, 'w') do |f|
          f.write result.to_json
        end
      else
        assert(false, "Tests should have a result to compare to")
      end
    end
  end
  
end
