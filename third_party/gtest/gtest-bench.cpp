#include <gtest/gtest.h>
#include <gtest/gtest-bench.h>

#include <helper/PapiTracer.h>

#include <iostream>
#include <limits>
#include <math.h>
#include <string>
#include <map>

typedef class values {
 private:
  double tmp_stddev;
 public:
  double stddev, mean, min, max;
  size_t num_values;
  values() :
      tmp_stddev(0), stddev(0), mean(0),
      min(std::numeric_limits<PapiTracer::result_t>::max()), max(0),
      num_values(0) {}

  void updateWith(double value) {
    min = value < min ? value : min;
    max = value > max ? value : max;

    // Code derived from http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#On-line_algorithm
    ++num_values;
    auto delta = value - mean;
    mean += delta / num_values;
    tmp_stddev += delta * (value - mean);

    auto variance = tmp_stddev / (num_values - 1);
    stddev = sqrt(variance);
  }
} values_t;

void testing::Benchmark::logValue(const std::string& key,
                                  double value) const {

    std::stringstream tmp;
    tmp << value;
    RecordProperty(key.c_str(), tmp.str().c_str());
}

void testing::Benchmark::TestBody()
{
  for (int i=0; i < WarmUp(); ++i)
  {
    BenchmarkSetUp();
    BenchmarkBody();
    BenchmarkTearDown();
  }

  std::map<std::string, values_t> counters { {"PAPI_TOT_CYC", {} }, {PapiEvent(), {}}, {"PAPI_L1_DCM", {}} };    
  for(int i=0; i < NumIterations(); ++i)
  {
    try {
      BenchmarkSetUp();
            
      PapiTracer pt;
      for (const auto& kv: counters) {
        pt.addEvent(kv.first);
      }
      
      pt.start();
            
      BenchmarkBody();

      pt.stop();

      BenchmarkTearDown();

      for (auto& kv: counters) {
        kv.second.updateWith(pt.value(kv.first));
      }

    } catch (const std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }

  

  for (auto& kv: counters) {
    const auto& key = kv.first;
    auto& values = kv.second;
    logValue(key+"_MIN", values.min);
    logValue(key+"_MAX", values.max);
    logValue(key+"_MEAN", values.mean);
    logValue(key+"_STDDEV", values.stddev);
  }
}
