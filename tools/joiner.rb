require 'json'

class Joiner

  JOIN_NAME = "RadixJoin"

  def initialize(f)
    @plan = JSON::load(File.new(f))
  end


  def parse()
    join_op = @plan["operators"].select{|k,v| v["type"] == JOIN_NAME}

    join_op.each do |k, join|
      # Get all infos
      probe_par = join["probe_par"]
      hash_par = join["hash_par"]
      join_par = 16
      bits1 = join["bits1"]
      bits2 = join["bits2"]
      fields = join["fields"]

      # Find in edges
      in_edges = @plan["edges"].select {|e| e[1] == k }.collect {|x| x[0] }
      out_edges = @plan["edges"].select {|e| e[0] == k }.collect {|x| x[1] }

      probe = build_probe_side("#{k}_probe_", fields[0], probe_par, bits1, bits2, in_edges[0], out_edges)
      hash = build_hash_side("#{k}_hash_", fields[0], hash_par, bits1, bits2, in_edges[1], out_edges)

      ops = probe[0].merge(hash[0])
      # Now connect the join

      # We have as many partitions as we bits in the first pass have
      partitions = 1 << bits1

      # Now remove the virtual join operator and remove all containing
      # edges
      @plan["operators"].delete(k)
      @plan["edges"].select! {|e| !e.include?(k)}

      ops[build_name(JOIN_NAME, k, "union")] = {
        "type" => "UnionScan"
      }

      edges = probe[1] + hash[1]

      # All Partitions
      all_partitions = (0..(partitions-1)).step(1).to_a

      join_par.times do |jp|

        first = all_partitions.size / join_par * jp

        # Calculate and override if necessary
        last = (all_partitions.size / join_par * (jp + 1)) - 1
        last = all_partitions.size - 1 if (jp + 1) == all_partitions.size

        ops[build_name(JOIN_NAME, k, jp)] = {
          "type" => "NestedLoopEquiJoin",
          "bits1" => bits1,
          "bits2" => bits2,
          "partitions" => all_partitions[first..last] # add missing partitions
        }

        edges << [in_edges[0], build_name(JOIN_NAME, k, jp)]
        edges << [probe[1][-1][1], build_name(JOIN_NAME, k, jp)]
        edges << [probe[1][-2][1], build_name(JOIN_NAME, k, jp)]
        edges << [in_edges[1], build_name(JOIN_NAME, k, jp)]
        edges << [hash[1][-1][1], build_name(JOIN_NAME, k, jp)]
        edges << [hash[1][-2][1], build_name(JOIN_NAME, k, jp)]
        edges << [build_name(JOIN_NAME, k, jp), build_name(JOIN_NAME, k, "union")]
      end

      edges << [build_name(JOIN_NAME, k, "union"), out_edges[0]]
      
      @plan["operators"].merge!(ops)
      @plan["edges"] += edges
    end

    puts JSON::pretty_generate(@plan)
  end

  def build_name(base, name, part=nil)
    unless part.nil?
      "#{base}_#{name}__#{part}"
    else
      "#{base}_#{name}"
    end
  end

  def build_hash_side(base_name, field, hash_par, bits1, bits2, in_edge, out_edges)

    histogram = {"#{base_name}_histogram" =>  {"type" => "Histogram", "fields" => [field], "bits" => bits1}}
    create_radix_table = {"#{base_name}_create_radix_table" => {"type" => "CreateRadixTable"}}
    create_radix_table2 = {"#{base_name}_create_radix_table2" => {"type" => "CreateRadixTable"}}
    prefix_sum = {"#{base_name}_prefix_sum" => {"type" => "PrefixSum"}}
    radix_cluster_p1 = {"#{base_name}_radix_cluster_p1" => {"type" => "RadixCluster", "fields" => [field], "bits" => bits1}}
    histogram_p2 = {"#{base_name}_histogram_p2" => {"type" => "Histogram2ndPass", "fields" => [field], "bits" => bits1, "bits2" => bits2, "sig2" => bits1}}
    prefix_sum_p2 = {"#{base_name}_prefix_sum_p2" => {"type" => "PrefixSum"}}
    radix_cluster_p2 = {"#{base_name}_radix_cluster_p2" => {"type" => "RadixCluster2ndPass", "bits" => bits1, "bits2" => bits2, "sig2" => bits1}}
    merge_prefix_sum = {"#{base_name}_merge_prefix_sum" => {"type" => "MergePrefixSum"}}
    barrier = {"#{base_name}_barrier" => {"type" => "Barrier", "fields" => [0]}}

    result = {}
    result.merge!(split_op(histogram, hash_par))
    result.merge!(split_op(prefix_sum, hash_par))
    result.merge!(split_op(radix_cluster_p1, hash_par))
    result.merge!(split_op(histogram_p2, hash_par))
    result.merge!(split_op(prefix_sum_p2, hash_par))
    result.merge!(split_op(radix_cluster_p2, hash_par))
    result.merge!(merge_prefix_sum)
    result.merge!(create_radix_table)
    result.merge!(create_radix_table2)
    result.merge!(barrier)

    # Then define the edges
    edges = []

    # There is an edge from input to create cluster table and for the second pass
    edges << [in_edge, "#{base_name}_create_radix_table"]
    edges << [in_edge, "#{base_name}_create_radix_table2"]
    hash_par.times do |i|
      # the input goes to all histograms
      edges << [in_edge, "#{base_name}_histogram__#{i}"]
      # All equal histograms go to all prefix sums
      hash_par.times do |j|
        edges << ["#{base_name}_histogram__#{i}", "#{base_name}_prefix_sum__#{j}"]
      end

      # And from each input there is a link to radix clustering
      edges << [in_edge, "#{base_name}_radix_cluster_p1__#{i}"]
      # From create radix table to radix cluster for the second pass as well
      edges << ["#{base_name}_create_radix_table", "#{base_name}_radix_cluster_p1__#{i}"]
      

      # From each prefix sum there is a link to radix clustering
      edges << ["#{base_name}_prefix_sum__#{i}", "#{base_name}_radix_cluster_p1__#{i}"]

      # now comes the second pass which is like the first only a litte
      # more complicated
      hash_par.times do |j|
        # We need an explicit barrier here to avoid that a histogram is calculated before all other
        # first pass radix clusters finished
        edges << [build_name(base_name, "radix_cluster_p1", i), build_name(base_name, "histogram_p2", j)]
        edges << [build_name(base_name, "histogram_p2", i), build_name(base_name, "prefix_sum_p2", j)]
      end

      # This builds up the second pass radix cluster, attention order matters
      edges << [build_name(base_name, "radix_cluster_p1", i), build_name(base_name, "radix_cluster_p2", i)]
      edges << ["#{base_name}_create_radix_table2", "#{base_name}_radix_cluster_p2__#{i}"]
      edges << [build_name(base_name, "prefix_sum_p2", i), build_name(base_name, "radix_cluster_p2", i)]
      edges << [build_name(base_name, "prefix_sum_p2", i), build_name(base_name, "merge_prefix_sum")]
      edges << [build_name(base_name, "radix_cluster_p2", i), build_name(base_name, "barrier")]
    end

    [result, edges]
  end

  def build_probe_side(base_name, field, probe_par, bits1, bits2, in_edge, out_edges)

    histogram = {"#{base_name}_histogram" => {"type" => "Histogram", "fields" => [field], "bits" => bits1}}
    create_radix_table = {"#{base_name}_create_radix_table" =>  {"type" => "CreateRadixTable"}}
    prefix_sum = {"#{base_name}_prefix_sum" =>  {"type" => "PrefixSum"}}
    radix_cluster_p1 = {"#{base_name}_radix_cluster_p1" => {"type" => "RadixCluster", "fields" => [field], "bits" => bits1}}
    merge_prefix_sum = {"#{base_name}_merge_prefix_sum" => {"type" => "MergePrefixSum"}}
    barrier = {"#{base_name}_barrier" => {"type" => "Barrier", "fields" => [0]}}

    # First define the plan ops
    result = {}
    result.merge!(split_op(histogram, probe_par))
    result.merge!(split_op(prefix_sum, probe_par))
    result.merge!(split_op(radix_cluster_p1, probe_par))
    result.merge!(create_radix_table)
    result.merge!(merge_prefix_sum)
    result.merge!(barrier)

    # Then define the edges
    edges = []

    # There is an edge from input to create cluster table
    edges << [in_edge, "#{base_name}_create_radix_table"]
    probe_par.times do |i|
      # the input goes to all histograms
      edges << [in_edge, "#{base_name}_histogram__#{i}"]
      # All equal histograms go to all prefix sums
      probe_par.times do |j|
        edges << ["#{base_name}_histogram__#{i}", "#{base_name}_prefix_sum__#{j}"]
      end

      # And from each input there is a link to radix clustering
      edges << [in_edge, "#{base_name}_radix_cluster_p1__#{i}"]
      # From create radix table to radix cluster
      edges << ["#{base_name}_create_radix_table", "#{base_name}_radix_cluster_p1__#{i}"]
      # From each prefix sum there is a link to radix clustering
      edges << ["#{base_name}_prefix_sum__#{i}", "#{base_name}_radix_cluster_p1__#{i}"]

      #  Merge all prefix sums
      edges << ["#{base_name}_prefix_sum__#{i}", "#{base_name}_merge_prefix_sum"]
      # Barrier on all radix_clusters
      edges << ["#{base_name}_radix_cluster_p1__#{i}", "#{base_name}_barrier"]
    end
    
    [result, edges]
  end

  def deep_copy(o)
    Marshal.load(Marshal.dump(o))
  end

  def split_op(op, num)
    result = {}
    num.times do |i|
      tmp = deep_copy(op)
      old_key = tmp.keys[0]
      tmp[old_key]["part"] = i
      tmp[old_key]["numParts"] = num
      key = tmp.keys[0] + "__#{i}"
      result.merge!({key => tmp[old_key]})
    end
    result
  end


end



j = Joiner.new(ARGV[0])
j.parse()
