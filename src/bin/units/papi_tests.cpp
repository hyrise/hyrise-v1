#include "testing/base_test.h"

#include "helper/PapiTracer.h"

class PapiTests : public ::hyrise::Test {};

void work() {
    for (volatile auto i=0; i<10; i++) { };
}

TEST_F(PapiTests, trace_basic) {
  PapiTracer pt;
  pt.addEvent("PAPI_TOT_CYC");
  pt.start();
  work();
  pt.stop();

#ifdef USE_PAPI_TRACE
  ASSERT_GT(pt.value("PAPI_TOT_CYC"), 0);
#endif
}

TEST_F(PapiTests, trace_nested_with_disabled_inner) {
  PapiTracer outer;
  outer.addEvent("PAPI_TOT_CYC");
  outer.start();

  PapiTracer inner;
  inner.addEvent("NO_PAPI");
  inner.start();

  work();

  inner.stop();
  outer.stop();
}

TEST_F(PapiTests, trace_nested_with_enabled_inner) {
  PapiTracer outer;
  outer.addEvent("PAPI_TOT_CYC");
  outer.start();

  PapiTracer inner;
  inner.addEvent("PAPI_TOT_CYC");
  inner.addEvent("NO_PAPI");
  inner.start();

  work();

  inner.stop();
#ifdef USE_PAPI_TRACE
  ASSERT_EQ(inner.value("PAPI_TOT_CYC"), 0) << "Disabled counters should return zero";
#endif
  outer.stop();

#ifdef USE_PAPI_TRACE
  ASSERT_GT(outer.value("PAPI_TOT_CYC"), 0);
#endif
}
