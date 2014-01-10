// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include "storage/OrderIndifferentDictionary.h"
#include "storage/OrderPreservingDictionary.h"
#include "storage/PassThroughDictionary.h"

namespace hyrise {
namespace storage {

class DictionaryTest : public Test {};

TEST_F(DictionaryTest, PassThroughDictionaries) {
  
  PassThroughDictionary<hyrise_int32_t> d;
  ASSERT_EQ(99, d.addValue(99));
  ASSERT_FALSE(d.isOrdered());
  
  ASSERT_TRUE(d.isValueIdValid(382));
  ASSERT_TRUE(d.valueExists(83821));

  ASSERT_EQ(876, d.getValueIdForValue(876));
  ASSERT_EQ(876, d.getValueForValueId(876));
  
  PassThroughDictionary<hyrise_float_t> d2;

  float o = 84.2638;
  auto v = *reinterpret_cast<value_id_t*>(&o);


  ASSERT_EQ(v, d2.addValue(o));
  ASSERT_FALSE(d2.isOrdered());
  
  ASSERT_TRUE(d2.isValueIdValid(382));
  ASSERT_TRUE(d2.valueExists(83821));


  ASSERT_EQ(v, d2.getValueIdForValue(o));
  ASSERT_EQ(o, d2.getValueForValueId(v));

}

TEST_F(DictionaryTest, create_dictionary) {
  OrderPreservingDictionary<int> d;

  value_id_t t1 = d.addValue(10);
  value_id_t t2 = d.addValue(20);

  ASSERT_EQ(0u, t1);
  ASSERT_EQ(1u, t2);
}

TEST_F(DictionaryTest, check_validity) {
  OrderPreservingDictionary<int> d;

  d.addValue(10);
  d.addValue(20);

  ASSERT_TRUE(d.isValueIdValid(1));
  ASSERT_TRUE(!d.isValueIdValid(99));
}

TEST_F(DictionaryTest, check_existence) {
  OrderPreservingDictionary<int> d;

  d.addValue(10);
  d.addValue(20);

  value_id_t t = d.getValueIdForValue(10);
  ASSERT_EQ(0u, t);

  t = d.getValueIdForValue(99);
  ASSERT_EQ(2u, t);

  ASSERT_TRUE(!d.isValueIdValid(2));
  ASSERT_TRUE(!d.valueExists(99));
}

TEST_F(DictionaryTest, check_order) {
  OrderPreservingDictionary<int> d;

  d.addValue(10);
  d.addValue(20);

  ASSERT_TRUE(d.isOrdered());
}

TEST_F(DictionaryTest, order_preserving_iterator_test) {
  OrderPreservingDictionary<int> dict;
  dict.addValue(1);
  dict.addValue(3);
  dict.addValue(5);

  OrderPreservingDictionary<int>::iterator it = dict.begin();

  ASSERT_EQ(3u, dict.size());

  ASSERT_EQ(1, *it);
  it++;
  ASSERT_EQ(3, *it);
  ++it;
  ASSERT_EQ(5, *it);
  ++it;

  ASSERT_TRUE(it == dict.end());
}

TEST_F(DictionaryTest, order_indifferent_iterator_test) {
  OrderIndifferentDictionary<int> dict;
  dict.addValue(1);
  dict.addValue(5);
  dict.addValue(3);

  OrderIndifferentDictionary<int>::iterator it = dict.begin();

  ASSERT_EQ(3u, dict.size());

  ASSERT_EQ(1, *it);
  it++;
  ASSERT_EQ(3, *it);
  ++it;
  ASSERT_EQ(5, *it);
  ++it;

  ASSERT_TRUE(it == dict.end());

}

TEST_F(DictionaryTest, order_indifferent_string_iterator_test) {
  OrderIndifferentDictionary<std::string> dict;
  dict.addValue("asd");
  dict.addValue("ufo");
  dict.addValue("ekg");

  OrderIndifferentDictionary<std::string>::iterator it = dict.begin();

  ASSERT_EQ(3u, dict.size());

  ASSERT_EQ("asd", *it);
  it++;
  ASSERT_EQ("ekg", *it);
  ++it;
  ASSERT_EQ("ufo", *it);
  ++it;

  ASSERT_TRUE(it == dict.end());
}

TEST_F(DictionaryTest, order_preserving_string_exists) {
  OrderPreservingDictionary<std::string> dict;
  dict.addValue("a");
  dict.addValue("b");


  ASSERT_TRUE(dict.valueExists("a"));
  ASSERT_FALSE(dict.valueExists("c"));
  ASSERT_FALSE(dict.valueExists("321"));

}

} } // namepsace hyrise::storage

