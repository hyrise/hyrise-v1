// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include "storage/CompoundValueKeyBuilder.h"
#include "storage/CompoundValueIdKeyBuilder.h"

namespace hyrise {
namespace storage {

class CompoundValueKeyBuilderTest : public ::hyrise::Test {};
class CompoundValueIdKeyBuilderTest : public ::hyrise::Test {};

TEST_F(CompoundValueKeyBuilderTest, twoInts) {

  CompoundValueKeyBuilder builder;  // 0  0  0  0  0  0  0  0 ...
  builder.add((hyrise_int_t)13);  // 13 0  0  0  0  0  0  0 ...
  builder.add((hyrise_int_t)37);  // 13 0  37 0  0  0  0  0 ...
  compound_value_key_t key = builder.get();


  CompoundValueKeyBuilder builder2;
  builder2.add((hyrise_int_t)0);
  builder2.add((hyrise_int_t)37);
  compound_value_key_t key2 = builder2.get();


  CompoundValueKeyBuilder builder3;
  builder3.add((hyrise_int_t)0);
  builder3.add((hyrise_int_t)38);
  compound_value_key_t key3 = builder3.get();

  ASSERT_TRUE(key2 < key);
  ASSERT_TRUE(key2 < key3);
}



TEST_F(CompoundValueKeyBuilderTest, neg_pos_value) {

  CompoundValueKeyBuilder b1;
  b1.add((hyrise_int_t)1);
  b1.add((hyrise_int_t) - 1);
  compound_value_key_t key1 = b1.get();

  CompoundValueKeyBuilder b2;
  b2.add((hyrise_int_t)1);
  b2.add((hyrise_int_t)2);
  compound_value_key_t key2 = b2.get();

  ASSERT_TRUE(key1 < key2);
}


TEST_F(CompoundValueKeyBuilderTest, neg_neg_value) {

  CompoundValueKeyBuilder b1;
  b1.add((hyrise_int_t) - 1);
  b1.add((hyrise_int_t) - 1);
  compound_value_key_t key1 = b1.get();

  CompoundValueKeyBuilder b2;
  b2.add((hyrise_int_t) - 2);
  b2.add((hyrise_int_t) - 2);
  compound_value_key_t key2 = b2.get();

  ASSERT_TRUE(key1 > key2);
}

TEST_F(CompoundValueKeyBuilderTest, negativePositiveSameDigit) {

  CompoundValueKeyBuilder b1;
  b1.add((hyrise_int_t) - 4);
  compound_value_key_t key1 = b1.get();

  CompoundValueKeyBuilder b2;
  b2.add((hyrise_int_t)4);
  compound_value_key_t key2 = b2.get();

  ASSERT_TRUE(key1 < key2);
}


TEST_F(CompoundValueKeyBuilderTest, tooLong) {
  // TODO: Make builder aware of keylength, e.g. Typedef to keylength or make a parameter.
  CompoundValueKeyBuilder builder;
  for (int i = 0; i < 5; i++)
    builder.add((hyrise_int_t)1);
  ASSERT_THROW(builder.add((hyrise_int_t)13), std::runtime_error);
}

TEST_F(CompoundValueKeyBuilderTest, intAndString) {

  CompoundValueKeyBuilder builder;  // 0  0  0  0  0  0  0  0 ...
  builder.add((hyrise_int_t)15);  // 15 0  0  0  0  0  0  0 ...
  std::string test("foo1");
  builder.add(test);  // 15 0  f  o  o  0  0  0 ...
  compound_value_key_t key = builder.get();


  CompoundValueKeyBuilder builder2;  // 0  0  0  0  0  0  0  0 ...
  builder2.add((hyrise_int_t)15);  // 15 0  0  0  0  0  0  0 ...
  std::string test2("foo2");
  builder2.add(test2);  // 15 0  f  o  o  0  0  0 ...
  compound_value_key_t key2 = builder2.get();


  ASSERT_TRUE(key < key2);

  // ASSERT_EQ(15,  key[0]);
  // ASSERT_EQ(0,   key[1]);
  // ASSERT_EQ('f', key[2]);
  // ASSERT_EQ('o', key[3]);
  // ASSERT_EQ('o', key[4]);
  // for(int i = 5; i < compound_value_key_size; i++) ASSERT_EQ(0, key[i]);
}

TEST_F(CompoundValueKeyBuilderTest, maxIntGreaterThanMinInt) {

  CompoundValueKeyBuilder b1;
  b1.add((hyrise_int_t)std::numeric_limits<hyrise_int_t>::max());
  compound_value_key_t key1 = b1.get();

  CompoundValueKeyBuilder b2;
  b2.add((hyrise_int_t)std::numeric_limits<hyrise_int_t>::min());
  compound_value_key_t key2 = b2.get();

  ASSERT_TRUE(key1 > key2);
}

TEST_F(CompoundValueKeyBuilderTest, maxMinMaxGreaterThanMinMaxMin) {

  CompoundValueKeyBuilder b1;
  b1.add((hyrise_int_t)std::numeric_limits<hyrise_int_t>::max());
  b1.add((hyrise_int_t)std::numeric_limits<hyrise_int_t>::min());
  b1.add((hyrise_int_t)std::numeric_limits<hyrise_int_t>::max());
  compound_value_key_t key1 = b1.get();

  CompoundValueKeyBuilder b2;
  b2.add((hyrise_int_t)std::numeric_limits<hyrise_int_t>::min());
  b2.add((hyrise_int_t)std::numeric_limits<hyrise_int_t>::max());
  b2.add((hyrise_int_t)std::numeric_limits<hyrise_int_t>::min());
  compound_value_key_t key2 = b2.get();

  ASSERT_TRUE(key1 > key2);
}

TEST_F(CompoundValueIdKeyBuilderTest, threeValueIds) {

  CompoundValueIdKeyBuilder builder;
  builder.add(12, 15);
  builder.add(3, 16);
  builder.add(0, 1);
  builder.add(34, 50);


  CompoundValueIdKeyBuilder builder2;
  builder2.add(12, 15);
  builder2.add(3, 16);
  builder2.add(0, 1);
  builder2.add(35, 50);


  ASSERT_TRUE(builder.get() < builder2.get());
  // ASSERT_EQ(3250847744, (uint32_t)(builder.get() >> 32));
}

TEST_F(CompoundValueIdKeyBuilderTest, upperbounds) {

  CompoundValueIdKeyBuilder lower;
  CompoundValueIdKeyBuilder upper;
  lower.add(12, 15);
  upper.add(12, 15);
  ASSERT_TRUE(lower.get() == upper.get());

  lower.add(3, 16);
  upper.add(3, 16);
  ASSERT_TRUE(lower.get() == upper.get());

  lower.add(0, 1);
  upper.add(0, 1);
  ASSERT_TRUE(lower.get() == upper.get());


  lower.add(0, 50);
  upper.add(2, 50);
  ASSERT_TRUE(lower.get() < upper.get());

  // ASSERT_EQ(3250847744, (uint32_t)(builder.get() >> 32));
}

TEST_F(CompoundValueIdKeyBuilderTest, dictSizeMaxInt) {

  CompoundValueIdKeyBuilder builder;
  builder.add(2, std::numeric_limits<size_t>::max());

  CompoundValueIdKeyBuilder builder2;
  builder2.add(3, std::numeric_limits<size_t>::max());


  ASSERT_TRUE(builder.get() < builder2.get());
}

TEST_F(CompoundValueIdKeyBuilderTest, DISABLED_dictSizeMinInt) {

  CompoundValueIdKeyBuilder builder;
  builder.add(2, std::numeric_limits<size_t>::min());

  CompoundValueIdKeyBuilder builder2;
  builder2.add(3, std::numeric_limits<size_t>::min());


  ASSERT_TRUE(builder.get() < builder2.get());
}

TEST_F(CompoundValueIdKeyBuilderTest, dictSizeMaxMinMaxTooBig) {

  CompoundValueIdKeyBuilder builder;
  builder.add(5, std::numeric_limits<size_t>::max());
  builder.add(5, std::numeric_limits<size_t>::min());
  ASSERT_THROW(builder.add(5, std::numeric_limits<size_t>::max()), std::runtime_error);
}

TEST_F(CompoundValueIdKeyBuilderTest, dictSizeMinMaxMin) {

  CompoundValueIdKeyBuilder builder;
  builder.add(5, std::numeric_limits<size_t>::min());
  builder.add(5, std::numeric_limits<size_t>::max());
  builder.add(4, std::numeric_limits<size_t>::min());

  CompoundValueIdKeyBuilder builder2;
  builder2.add(5, std::numeric_limits<size_t>::min());
  builder2.add(5, std::numeric_limits<size_t>::max());
  builder2.add(5, std::numeric_limits<size_t>::min());


  ASSERT_TRUE(builder.get() == builder2.get());
}

TEST_F(CompoundValueIdKeyBuilderTest, DISABLED_dictSizeMinIntEqThanMaxInt) {

  CompoundValueIdKeyBuilder builder;
  builder.add(2, std::numeric_limits<size_t>::min());

  CompoundValueIdKeyBuilder builder2;
  builder2.add(2, std::numeric_limits<size_t>::max());

  ASSERT_TRUE(builder.get() == builder2.get());
}

TEST_F(CompoundValueIdKeyBuilderTest, dictSizeMinIntSmallerThanHalfMaxInt) {

  CompoundValueIdKeyBuilder builder;
  builder.add(2, std::numeric_limits<size_t>::min());

  CompoundValueIdKeyBuilder builder2;
  builder2.add(2, std::numeric_limits<size_t>::max() / 2);

  ASSERT_TRUE(builder.get() < builder2.get());
}

TEST_F(CompoundValueIdKeyBuilderTest, valueIdIsGreaterThanDictSize) {

  CompoundValueIdKeyBuilder builder;
  builder.add(12, 10);
  builder.add(14, 10);
  builder.add(18, 10);
  builder.add(25, 10);

  CompoundValueIdKeyBuilder builder2;
  builder2.add(12, 10);
  builder2.add(14, 10);
  builder2.add(18, 10);
  builder2.add(26, 10);

  ASSERT_TRUE(builder.get() < builder2.get());
}

TEST_F(CompoundValueIdKeyBuilderTest, valueIdIsMaxInt) {

  CompoundValueIdKeyBuilder builder;
  builder.add(std::numeric_limits<hyrise_int_t>::max() - 1, 3);

  CompoundValueIdKeyBuilder builder2;
  builder2.add(std::numeric_limits<hyrise_int_t>::max(), 3);


  ASSERT_TRUE(builder.get() < builder2.get());
}

TEST_F(CompoundValueIdKeyBuilderTest, valueIdIsMinInt) {

  CompoundValueIdKeyBuilder builder;
  builder.add(std::numeric_limits<hyrise_int_t>::min() + 1, 3);

  CompoundValueIdKeyBuilder builder2;
  builder2.add(std::numeric_limits<hyrise_int_t>::min(), 3);


  ASSERT_TRUE(builder.get() > builder2.get());
}

TEST_F(CompoundValueIdKeyBuilderTest, valueIdMinLessThanMax) {

  CompoundValueIdKeyBuilder builder;
  builder.add(std::numeric_limits<hyrise_int_t>::min(), 10);

  CompoundValueIdKeyBuilder builder2;
  builder2.add(std::numeric_limits<hyrise_int_t>::max(), 10);


  ASSERT_TRUE(builder.get() < builder2.get());
}

TEST_F(CompoundValueIdKeyBuilderTest, valueIdMinMaxMinLessThanMaxMinMax) {

  CompoundValueIdKeyBuilder builder;
  builder.add(std::numeric_limits<hyrise_int_t>::min(), 10);
  builder.add(std::numeric_limits<hyrise_int_t>::max(), 10);
  builder.add(std::numeric_limits<hyrise_int_t>::min(), 10);

  CompoundValueIdKeyBuilder builder2;
  builder2.add(std::numeric_limits<hyrise_int_t>::max(), 10);
  builder2.add(std::numeric_limits<hyrise_int_t>::min(), 10);
  builder2.add(std::numeric_limits<hyrise_int_t>::max(), 10);


  ASSERT_TRUE(builder.get() < builder2.get());
}

TEST_F(CompoundValueIdKeyBuilderTest, sameValueIdDifferentDictsize) {

  CompoundValueIdKeyBuilder builder;
  builder.add(4, 7);

  CompoundValueIdKeyBuilder builder2;
  builder2.add(4, 10);


  ASSERT_TRUE(builder.get() > builder2.get());
}

TEST_F(CompoundValueIdKeyBuilderTest, maxValueIdEqualsDictSize) {

  CompoundValueIdKeyBuilder builder;
  builder.add(std::numeric_limits<hyrise_int_t>::max() - 1, std::numeric_limits<hyrise_int_t>::max());

  CompoundValueIdKeyBuilder builder2;
  builder2.add(std::numeric_limits<hyrise_int_t>::max(), std::numeric_limits<hyrise_int_t>::max());


  ASSERT_TRUE(builder.get() < builder2.get());
}

TEST_F(CompoundValueIdKeyBuilderTest, tooLong) {
  CompoundValueIdKeyBuilder builder;
  for (int i = 0; i < 21; i++)
    builder.add(1, 7);
  ASSERT_THROW(builder.add(1, 7), std::runtime_error);
}
}
}
