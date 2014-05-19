// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include <limits>

#include "helper/checked_cast.h"

#include "io/shortcuts.h"

#include "storage/storage_types.h"
#include "storage/BitCompressedVector.h"
#include "storage/BaseAttributeVector.h"
#include "storage/FixedLengthVector.h"
#include "storage/bat_vector.h"
#include "storage/model/attribute_model.h"
#include <storage/TableBuilder.h>
#include "storage/Store.h"

namespace hyrise {
namespace storage {

TEST(AttributeModel, simple) {

  hyrise::storage::atable_ptr_t input = io::Loader::shortcuts::load("test/tables/model_1.tbl");
  const auto& store = std::dynamic_pointer_cast<Store>(input);
  const auto& avs = store->getAttributeVectors(0);
  const auto& flv = avs.front();
  model::AttributeModelCallback amc(store);

  const auto& mostFrequent = amc.mostFrequent(checked_pointer_cast<FixedLengthVector<value_id_t>>(flv.attribute_vector), flv.attribute_offset);

  EXPECT_EQ(0ul, std::get<0>(mostFrequent));
  EXPECT_EQ(input->size() - 3, std::get<1>(mostFrequent));

}

TEST(AttributeModel, column_model) {

  hyrise::storage::atable_ptr_t input = io::Loader::shortcuts::load("test/tables/model_1.tbl");
  const auto& store = std::dynamic_pointer_cast<Store>(input);
  model::AttributeModelCallback amc(store);

  const auto& new_av= amc(1);
  const auto& comp = checked_pointer_cast<BATVector<value_id_t, true>>(new_av);
  ASSERT_TRUE(nullptr != comp);

  const auto& new_av2= amc(1);
  const auto& comp2 = checked_pointer_cast<BATVector<value_id_t, true>>(new_av2);
  ASSERT_TRUE(nullptr != comp2);

  const auto& new_av3= amc(1);
  const auto& comp3 = std::dynamic_pointer_cast<BATVector<value_id_t, true>>(new_av3);
  ASSERT_TRUE(nullptr == comp3);

  const auto& comp4 = checked_pointer_cast<FixedLengthVector<value_id_t>>(new_av3);
  ASSERT_TRUE(nullptr != comp4);

}


TEST(BatVectorTests, append_values ) {
  BATVector<uint64_t, true> bat(3);

  bat.push_back(2);
  EXPECT_EQ(1ul, bat.size());
  EXPECT_EQ(1ul, bat.compressed_size());
  bat.push_back(3);
  EXPECT_EQ(2ul, bat.size());
  EXPECT_EQ(1ul, bat.compressed_size());
  bat.push_back(0);
  EXPECT_EQ(3ul, bat.size());
  EXPECT_EQ(2ul, bat.compressed_size());
}

TEST(BatVectorTests, get_values ) {
  BATVector<uint64_t, true> bat(3);

  bat.push_back(2);
  bat.push_back(3);
  bat.push_back(0);

  EXPECT_EQ(0ul, bat[2]);
  EXPECT_EQ(2ul, bat[0]);
  EXPECT_EQ(3ul, bat[1]);

}

TEST(BatVectorTests, match_values ) {
  using bv = BATVector<uint64_t, true>;
  BATVector<uint64_t, true> bat(3);

  bat.push_back(2);
  bat.push_back(3);
  bat.push_back(0);
  bat.push_back(3);
  bat.push_back(2);

  auto res = bat.match<std::equal_to<bv::value_type>>(0, bat.size(), 2);
  EXPECT_EQ(2ul, res.size());
  EXPECT_EQ(0ul, res[0]);
  EXPECT_EQ(4ul, res[1]);

}



TEST(BitCompressedTests, set_retrieve_bits) {
  std::vector<uint64_t> bits{1, 2, 4, 8, 13};
  BitCompressedVector<value_id_t> tuples(5, 2, bits);
  tuples.resize(2);
  auto col = 0;
  for (auto bit : bits) {
    uint64_t maxval = (1 << bit) - 1;  // Maximum value that can be stored is 2^bit-1
    tuples.set(col, 0, 0);
    tuples.set(col, 1, maxval);
    EXPECT_EQ(0u, tuples.get(col, 0));
    EXPECT_EQ(maxval, tuples.get(col, 1));
    col++;
  }
}

TEST(BitCompressedTests, empty_size_does_not_change_with_reserve) {
  BitCompressedVector<value_id_t> tuples(1, 1, {1});
  ASSERT_EQ(0u, tuples.size());
  ASSERT_EQ(64u, tuples.capacity());
  tuples.reserve(3);
  ASSERT_EQ(0u, tuples.size());
  ASSERT_EQ(64u, tuples.capacity());
  tuples.reserve(65);
  ASSERT_EQ(0u, tuples.size());
  ASSERT_EQ(128u, tuples.capacity());
}

TEST(FixedLengthVectorTest, increment_test) {
  size_t cols = 1;
  size_t rows = 3;

  FixedLengthVector<value_id_t> tuples(cols, rows);
  tuples.resize(rows);
  EXPECT_EQ(0u, tuples.get(0, 0));
  EXPECT_EQ(0u, tuples.inc(0, 0));
  EXPECT_EQ(1u, tuples.get(0, 0));
  EXPECT_EQ(1u, tuples.atomic_inc(0, 0));
  EXPECT_EQ(2u, tuples.get(0, 0));
}
}
}  // namespace hyrise::storage
