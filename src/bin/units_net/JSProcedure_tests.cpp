// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <gtest/gtest.h>

#include "net/Router.h"
#include "net/RouteRequestHandler.h"

namespace hyrise {
namespace net {

class JSProcedureTests : public ::testing::Test {};

TEST_F(JSProcedureTests, dummy) { ASSERT_EQ(true, true); }
}
}
