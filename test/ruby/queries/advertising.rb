
require 'hyrise'

def plan
  ads = HyriseOperator.new("TableLoad", { :filename => 'advertising/data.txt',
                                   :header => "advertising/header.txt",
                                   :table => "ads" })


  scan = HyriseOperator.new("SimpleTableScan", {
                              :predicates => [
                                              {:type=> "EQ", :in => 0, :f => "website", :value => 3}
                                             ]
                            })

  scan.add_dependency ads

  build = HyriseOperator.new("HashBuild", { :fields => [0] })
  build.add_dependency scan

  build2 = HyriseOperator.new("HashBuild", { :fields => [3] })
  build2.add_dependency scan

  group = HyriseOperator.new("GroupByScan", {
                               :fields => [0, 3],
                               :functions => [{:type => 1, :field => 4}]
                             })
  
  group.add_dependency(ads)
  group.add_dependency(build)
  group.add_dependency(build2)
  
  group
end
