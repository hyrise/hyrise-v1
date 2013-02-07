# The websites is the scaling variable
WEBSITES = 10

# These are the fixed variables that will not be changed over time
LOCATION_ID = 1000
BANNER   = 30
VIEWS = [10000,200000]
CLICKS = 0.005
TRANSACTIONS = 0.5



def generate_days(output, scale_factor = 1, days = 1)
  of = File.open(output, "w+")
  days.times do |d|
    (WEBSITES * scale_factor).times do |w|
      LOCATION_ID.times do |l|
        BANNER.times do |b|
          v = rand(VIEWS[1]-VIEWS[0]) + VIEWS[0]
          c = (CLICKS * v).to_i
          t = (c * TRANSACTIONS).to_i

          of.puts("#{d}| #{w}| #{l}| #{b}| #{v}| #{c}| #{t}")

        end
      end
    end
  end
  of.close()
end


if ARGV.size < 3
  puts "Usage: generator scale_factor days output.file"
else
  scale_factor = ARGV[0].to_i
  days = ARGV[1].to_i
  generate_days(ARGV[2], scale_factor, days)
end
