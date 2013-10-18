// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "MinimalistPrinter.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "gtest/gtest.h"

struct PrinterImpl {
  std::map<std::string, std::vector<MinimalistPrinter::test_part_t> > _tests;
  std::string _current_test;
};

MinimalistPrinter::MinimalistPrinter() :
    _impl(new PrinterImpl) {}

void MinimalistPrinter::OnTestStart(const ::testing::TestInfo &test_info) {
  auto vp = test_info.value_param();
  auto tp = test_info.type_param();
  _impl->_current_test = std::string()
      + test_info.test_case_name() + "." + test_info.name()
      // Add type param or value param information when available
      + ((vp || tp) ? " " : "") + (vp ? vp: "") + (tp ? tp: "");
  printf("%s", _impl->_current_test.c_str());
  fflush(stdout);
}

void MinimalistPrinter::OnTestPartResult(const ::testing::TestPartResult &test_part_result) {
  if (test_part_result.failed()) {
    _impl->_tests[_impl->_current_test].push_back(test_part_result);
  }
}

void MinimalistPrinter::OnTestEnd(const ::testing::TestInfo &test_info) {
  size_t len = _impl->_current_test.length();
  // clear previously printed testname and replace with . or F
  printf("%s%s%s%s",
         std::string(len, '\b').c_str(),
         std::string(len, ' ' ).c_str(),
         std::string(len, '\b').c_str(),
         _impl->_tests[_impl->_current_test].size() > 0 ? "F" : ".");
  fflush(stdout);
}

void MinimalistPrinter::OnTestProgramEnd(const ::testing::UnitTest &unit) {
  auto &_tests = _impl->_tests;
  printf("\n");
  size_t failed_tests = std::accumulate(_tests.begin(), _tests.end(), 0u,
                                        [](size_t acc, decltype(*_tests.begin()) value) -> size_t {
                                          return acc + value.second.size();
                                        });
  for (const auto & kvs: _tests) {
    if (kvs.second.size())
      printf("%s\n", kvs.first.c_str());
    for (const auto & test_part_result: kvs.second) {
      printf("%s:%d: Failure\n%s\n",
             test_part_result.file_name(),
             test_part_result.line_number(),
             test_part_result.summary());
    }
  }
  printf("\n> %lu errors (out of %lu tests)\n", failed_tests, _tests.size());
}
