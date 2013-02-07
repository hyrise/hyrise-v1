
require 'hyrise'

def plan
  lineorder = HyriseOperator.new("TableLoad", { :filename => 'ssb/lineorder.tbl',
                                   :header => "ssb/lineorder.hd",
                                   :table => "lineorder",
                                   :unsafe => true})
  
  date = HyriseOperator.new("TableLoad", { :filename => 'ssb/date.tbl',
                                   :header => "ssb/date.hd",
                                   :table => "date",
                                   :unsafe => true})
  

  # Q1
  scan_date = HyriseOperator.new("SimpleTableScan", {
                                   :materializing => false,
                                   :predicates => [
                                                   {:type=> "EQ", :in => 0, :f => "D_YEAR", :value => 1993}
                                                  ]
                            })
  scan_date.add_dependency(date)
  
  
  scan_lo = HyriseOperator.new("SimpleTableScan", {
                                 :materializing => false,
                                 :predicates => [
                                                 {:type => "AND"},
                                                 {:type=> "LT", :in => 0, :f => "LO_QUANTITY", :value => 25},
                                                 {:type=> "LT", :in => 0, :f => "LO_DISCOUNT", :value => 3}
                                             ]
                            })
  scan_lo.add_dependency(lineorder)



  build = HyriseOperator.new("HashBuild", { :fields => ["D_DATEKEY"] })
  build.add_dependency(scan_date)


  probe = HyriseOperator.new("HashJoinProbe", {:fields => ["LO_ORDERDATE"]})
  probe.add_dependency(build)
  probe.add_dependency(scan_lo)


  group = HyriseOperator.new("GroupByScan", {
                               :functions => [{:type => 1, :field => "LO_EXTENDEDPRICE"}]
                             })
  
  group.add_dependency(probe)
  group
end
