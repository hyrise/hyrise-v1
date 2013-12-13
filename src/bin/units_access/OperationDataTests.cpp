#include "testing/test.h"

#include "access/system/OperationData-Impl.h"
#include "storage/AbstractResource.h"

namespace hyrise {
namespace access {

class CustomResource1 : public storage::AbstractResource {};
class CustomResource2 : public storage::AbstractResource {};

namespace {
auto cr1a = std::make_shared<CustomResource1>();
auto cr1b = std::make_shared<CustomResource1>();
auto cr2a = std::make_shared<CustomResource2>();
}

TEST(OperationDataTests, adding) {
  OperationData op;
  op.addResource(cr1a);
  op.addResource(cr1b);
  op.addResource(cr2a);
  EXPECT_EQ(op.all().size(), 3u);
  EXPECT_EQ(op.sizeOf<CustomResource1>(), 2u);
  EXPECT_EQ(op.sizeOf<CustomResource2>(), 1u);
}


TEST(OperationDataTests, setting) {
  OperationData op;
  op.addResource(cr1a);
  op.addResource(cr1b);
  op.addResource(cr2a);

  auto cr1_replacement = std::make_shared<CustomResource1>();
  auto cr2_replacement = std::make_shared<CustomResource2>();

  op.setNthOf<CustomResource1>(1, cr1_replacement);
  EXPECT_EQ(op.nthOf<CustomResource1>(1), cr1_replacement);

  op.setNthOf<CustomResource2>(0, cr2_replacement);
  EXPECT_EQ(op.nthOf<CustomResource2>(0), cr2_replacement);
}

TEST(OperationDataTests, retrieving) {
  OperationData op;
  op.addResource(cr1a);
  op.addResource(cr2a);
  op.addResource(cr1b);
  EXPECT_EQ(op.nthOf<CustomResource1>(0), cr1a);
  EXPECT_EQ(op.nthOf<CustomResource1>(1), cr1b);
  EXPECT_EQ(op.nthOf<CustomResource2>(0), cr2a);

  std::vector<std::shared_ptr<const CustomResource1>> c1_v {cr1a, cr1b};
  std::vector<std::shared_ptr<const CustomResource2>> c2_v {cr2a};
  EXPECT_EQ(c1_v, op.allOf<CustomResource1>());
  EXPECT_EQ(c2_v, op.allOf<CustomResource2>());
}

TEST(OperationDataTests, merging) {
  OperationData op;
  op.addResource(cr1a);

  OperationData op2;
  op2.addResource(cr2a);
  op2.addResource(cr1b);

  op.mergeWith(op2);
  EXPECT_EQ(2u, op.sizeOf<CustomResource1>());
  EXPECT_EQ(1u, op.sizeOf<CustomResource2>());
}

} } // namespace hyrise::access

