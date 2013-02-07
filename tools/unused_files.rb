#! /usr/bin/env ruby

unsused_files = []
storage = 0
Dir.glob("../test/**/*.tbl") do |fn|

  f = File.basename(fn)
  result = `fgrep -R "#{f}" ../src ../test/autojson`

  if result.size == 0
    unsused_files << [fn, File.stat(fn).size]
    storage += File.stat(fn).size
  end

end




unsused_files.sort! {|a,b| a[1] <=> b[1]}

unsused_files.each do |e|
  puts "#{e[0]}" # - #{(e[1]/1024.0/1024.0).round()}"
end

$stderr << "-" * 40 << "\n"
$stderr << storage / 1024.0 / 1024.0 << "\n"
