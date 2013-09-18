#include "testing/test.h"
#include "io/SimpleLogger.h"

#include <fstream>
#include <thread>
#include <vector>

namespace hyrise {
namespace io {

class SimpleLoggerTests : public ::hyrise::Test {};

void simpleLoggerTestWorker(uint64_t tx) {
  SimpleLogger::getInstance().logDictionary<int64_t>('a', 1, 2, 3);
  SimpleLogger::getInstance().logDictionary<float>('a', 1, 2.0f, 3);
  SimpleLogger::getInstance().logDictionary<std::string>('a', 1, "zwei", 3);
  std::vector<ValueId> vids;
  vids.push_back(ValueId(0, 0));
  vids.push_back(ValueId(1, 0));
  vids.push_back(ValueId(2, 0));
  SimpleLogger::getInstance().logValue(tx,'a',2,3,4,&vids);
  SimpleLogger::getInstance().logCommit(tx);
}

TEST_F(SimpleLoggerTests, simple_log_test) {
  std::vector<std::thread> threadpool;
  uint64_t tx = 1;

  // let 100 threads log concurrently into logfile
  for(int i=0; i<100; i++)
    threadpool.push_back(std::thread(simpleLoggerTestWorker, tx++));
  for(auto it=threadpool.begin(); it!=threadpool.end(); ++it)
    it->join();

  // now read the logfile and compare number of opening and closing brackets (should be the same)
  int64_t numLB=0, numRB=0;
  char c;
  std::ifstream is("logfile");
  while(is.good()) {
    c = is.get();
    if(c == '(')      ++numLB;
    else if(c == ')') ++numRB;
  }
  is.close();

  ASSERT_EQ(numRB, 600);
  ASSERT_EQ(numLB, 600);

}

}
}
