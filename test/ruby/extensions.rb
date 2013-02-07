require 'rubygems'
require 'minitest/unit'
require 'minitest/spec'

module HyriseHooks

  def self.startup
    if RUN_HYRISE_SERVER
      base = File.expand_path('../../../', __FILE__)
      @@olddir = Dir.pwd
      @@basedir = File.join(@@olddir, File.dirname(__FILE__))
      Dir.chdir base
      ENV['HYRISE_DB_PATH'] = 'test'
      @@server_proc = IO.popen("build/hyrise_server")
      sleep(1)
      $stderr << "Hyrise server launched\n"
    end
  end
  
  def self.shutdown   
    if RUN_HYRISE_SERVER   
      Process.kill 'INT', @@server_proc.pid
      $stderr << "\nHyrise server terminated\n"
      Dir.chdir @@olddir
    end
  end
end

class HyriseUnit < MiniTest::Unit

  def _run_suite(suite, type)
    begin
      suite.before_suite if suite != MiniTest::Spec
      super(suite, type)
    ensure
      suite.after_suite if suite != MiniTest::Spec
    end
  end
end

class HyriseTestCase < MiniTest::Unit::TestCase

  def self.before_suite
  end

  def self.after_suite
  end
  
end

MiniTest::Unit.runner = HyriseUnit.new
