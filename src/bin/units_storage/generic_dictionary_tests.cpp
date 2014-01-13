// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include <algorithm>

#include "storage/ConcurrentUnorderedDictionary.h"
#include "storage/OrderIndifferentDictionary.h"
#include "storage/OrderPreservingDictionary.h"
#include "storage/PassThroughDictionary.h"

namespace hyrise {
namespace storage {

class DictionaryTest : public Test {};

template <typename T>
class DictTests : public Test {};

typedef testing::Types <
  OrderIndifferentDictionary<hyrise_int_t>,OrderIndifferentDictionary<hyrise_int32_t>, OrderIndifferentDictionary<hyrise_float_t>, OrderIndifferentDictionary<hyrise_string_t>, 
  OrderPreservingDictionary<hyrise_int_t>, OrderPreservingDictionary<hyrise_int32_t>, OrderPreservingDictionary<hyrise_float_t>, OrderPreservingDictionary<hyrise_string_t>,
  ConcurrentUnorderedDictionary<hyrise_int_t>, ConcurrentUnorderedDictionary<hyrise_int32_t>, ConcurrentUnorderedDictionary<hyrise_float_t>, ConcurrentUnorderedDictionary<hyrise_string_t>
  > Dicts;

TYPED_TEST_CASE(DictTests, Dicts);

template <typename T>
std::vector<T> dict_values();

template <>
std::vector<hyrise_int_t> dict_values() {
  return {9, 0, 1, 4, 5, 6, 8};
}

template <>
std::vector<hyrise_int32_t> dict_values() {
  return {9, 0, 1, 4, 5, 6, 8};
}

template <>
std::vector<hyrise_float_t> dict_values() {
  return {2.1, 0, 1.1, 2.3, 3.7};
}

template <>
std::vector<hyrise_string_t> dict_values() {
  return {"f", "a", "b", "c", "d", "e"};
}

TYPED_TEST(DictTests, fill_ordered) {
  TypeParam p;
  auto values = dict_values<typename TypeParam::value_type>();
  std::sort(values.begin(), values.end());
  for (auto value: values) {
    p.addValue(value);
  }
  EXPECT_EQ(values.size(), p.size());
}

TYPED_TEST(DictTests, fill_unordered_where_possible) {
  TypeParam p;
  auto values = dict_values<typename TypeParam::value_type>();
  if (p.isOrdered())
    std::sort(values.begin(), values.end());
  for (auto value: values) {
    p.addValue(value);
  }

  for (auto value: values) {
    EXPECT_TRUE(p.valueExists(value));
  }
}

TYPED_TEST(DictTests, value_ids_reinstatement) {
  TypeParam p;
  auto values = dict_values<typename TypeParam::value_type>();
  if (p.isOrdered())
    std::sort(values.begin(), values.end());
  std::vector<value_id_t> valueids;
  for (auto value: values) {
    valueids.push_back(p.addValue(value));
  }
  EXPECT_EQ(values.size(), valueids.size()) << "Number of value ids should be equal to values";
  for (auto valueid: valueids) {
    EXPECT_TRUE(p.isValueIdValid(valueid));
    EXPECT_TRUE(std::count(values.begin(), values.end(), p.getValueForValueId(valueid)) == 1);
  }
}


TYPED_TEST(DictTests, test_nonexisting_value) {
  TypeParam p;
  auto values = dict_values<typename TypeParam::value_type>();
  if (p.isOrdered())
    std::sort(values.begin(), values.end());

  for (std::size_t i = 0u; i < values.size() - 2; ++i) {
    p.addValue(values[i]);
  }

  auto testval = values.back();
  EXPECT_FALSE(p.valueExists(testval));
}


#ifdef EXPENSIVE_ASSERTIONS
TYPED_TEST(DictTests, access_with_wrong_vid) {
  TypeParam p;
  auto values = dict_values<typename TypeParam::value_type>();
  auto vid = p.addValue(values.front());
  EXPECT_THROW(p.getValueForValueId(vid+1), std::out_of_range);
}
#endif

TYPED_TEST(DictTests, iterator_test) {
  TypeParam p;
  auto values = dict_values<typename TypeParam::value_type>();
  if (p.isOrdered())
    std::sort(values.begin(), values.end());
  for (auto value: values) {
    p.addValue(value);
  }
  EXPECT_TRUE(std::is_sorted(p.begin(), p.end())) << "Resulting iterator should be sorted";
}

} } // namespace hyrise::storage


