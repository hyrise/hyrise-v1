
require 'hyrise'

def plan
  companies = HyriseOperator.new("TableLoad", { :filename => 'tables/companies.tbl', :table => "companies" })
  employees = HyriseOperator.new("TableLoad", { :filename => 'tables/employees.tbl', :table => "employees" })

  build = HyriseOperator.new("HashBuild", { :fields => [0] })
  build.add_dependency companies
  
  probe = HyriseOperator.new("HashJoinProbe", { :fields => [1] })
  probe.add_dependency build
  probe.add_dependency employees
  
  probe
end
