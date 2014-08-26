// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include "io/shortcuts.h"

#include "storage/storage_types.h"

#include "access/Mult.h"
#include "access/Division.h"
#include "access/Mod.h"

namespace hyrise {
namespace access {

class MathTests : public AccessTest {
 public:
  static void SetUpTestCase() { inputTable = io::Loader::shortcuts::load("test/mathTestTable.tbl"); }
  static void TearDownTestCase() { inputTable.reset(); }

 protected:
  static storage::c_atable_ptr_t inputTable;
};

storage::c_atable_ptr_t MathTests::inputTable = nullptr;

TEST_F(MathTests, int_multiplication_1_test) {
  Mult mulInt;
  mulInt.addInput(inputTable);

  mulInt.addField(0);
  mulInt.addField(1);
  mulInt.setVType(IntegerType);
  mulInt.setFactor(1.0f);
  mulInt.setColName("MUL");

  mulInt.execute();
  const auto& result = mulInt.getResultTable();

  ASSERT_EQ(4u, result->size());
  ASSERT_EQ("MUL", result->nameOfColumn(5));
  ASSERT_TRUE(types::isCompatible(IntegerType, result->typeOfColumn(5)));
  ASSERT_EQ(246, result->getValue<storage::hyrise_int_t>(5, 0));
  ASSERT_EQ(0, result->getValue<storage::hyrise_int_t>(5, 1));
  ASSERT_EQ(0, result->getValue<storage::hyrise_int_t>(5, 2));
  ASSERT_EQ(63, result->getValue<storage::hyrise_int_t>(5, 3));
}

TEST_F(MathTests, int_multiplication_2_test) {
  Mult mulInt;
  mulInt.addInput(inputTable);

  mulInt.addField(1);
  mulInt.addField(2);
  mulInt.setVType(IntegerType);
  mulInt.setFactor(2.0f);
  mulInt.setColName("MUL");

  mulInt.execute();
  const auto& result = mulInt.getResultTable();

  ASSERT_EQ(80, result->getValue<storage::hyrise_int_t>(5, 0));
  ASSERT_EQ(70, result->getValue<storage::hyrise_int_t>(5, 1));
  ASSERT_EQ(3520, result->getValue<storage::hyrise_int_t>(5, 2));
  ASSERT_EQ(-42, result->getValue<storage::hyrise_int_t>(5, 3));
}

TEST_F(MathTests, int_multiplication_3_test) {
  Mult mulInt;
  mulInt.addInput(inputTable);

  mulInt.addField(2);
  mulInt.addField(3);
  mulInt.setVType(IntegerType);
  mulInt.setFactor(1.0f);
  mulInt.setColName("MUL");

  mulInt.execute();
  const auto& result = mulInt.getResultTable();

  ASSERT_EQ(22, result->getValue<storage::hyrise_int_t>(5, 0));
  ASSERT_EQ(0, result->getValue<storage::hyrise_int_t>(5, 1));
  ASSERT_EQ(0, result->getValue<storage::hyrise_int_t>(5, 2));
  ASSERT_EQ(-6, result->getValue<storage::hyrise_int_t>(5, 3));
}

TEST_F(MathTests, float_multiplication_test) {
  Mult mulFloat;
  mulFloat.addInput(inputTable);

  mulFloat.addField(2);
  mulFloat.addField(4);
  mulFloat.setVType(FloatType);
  mulFloat.setFactor(-1.1f);
  mulFloat.setColName("MUL");

  mulFloat.execute();
  const auto& result = mulFloat.getResultTable();

  ASSERT_TRUE(types::isCompatible(FloatType, result->typeOfColumn(5)));
  ASSERT_FLOAT_EQ(storage::hyrise_float_t(-48.4), result->getValue<storage::hyrise_float_t>(5, 0));
  ASSERT_FLOAT_EQ(storage::hyrise_float_t(-45.43), result->getValue<storage::hyrise_float_t>(5, 1));
  ASSERT_FLOAT_EQ(storage::hyrise_float_t(-2115.52), result->getValue<storage::hyrise_float_t>(5, 2));
  ASSERT_FLOAT_EQ(storage::hyrise_float_t(13.53), result->getValue<storage::hyrise_float_t>(5, 3));
}

TEST_F(MathTests, int_division_test) {
  Division divInt;
  divInt.addInput(inputTable);
  divInt.addField(0);
  divInt.addField(1);
  divInt.setVType(IntegerType);
  divInt.setColName("DIV");

  divInt.execute();
  const auto& result = divInt.getResultTable();

  ASSERT_TRUE(types::isCompatible(IntegerType, result->typeOfColumn(5)));
  ASSERT_EQ(61, result->getValue<storage::hyrise_int_t>(5, 0));
  ASSERT_EQ(0, result->getValue<storage::hyrise_int_t>(5, 1));
  ASSERT_EQ(0, result->getValue<storage::hyrise_int_t>(5, 2));
  ASSERT_EQ(1, result->getValue<storage::hyrise_int_t>(5, 3));
}

TEST_F(MathTests, float_division_test) {
  Division divFloat;
  divFloat.addInput(inputTable);
  divFloat.addField(3);
  divFloat.addField(4);
  divFloat.setVType(FloatType);
  divFloat.setColName("DIV");

  divFloat.execute();
  const auto& result = divFloat.getResultTable();

  ASSERT_TRUE(types::isCompatible(FloatType, result->typeOfColumn(5)));
  ASSERT_FLOAT_EQ(0.5f, result->getValue<storage::hyrise_float_t>(5, 0));
  ASSERT_FLOAT_EQ(0.f, result->getValue<storage::hyrise_float_t>(5, 1));
  ASSERT_FLOAT_EQ(0.f, result->getValue<storage::hyrise_float_t>(5, 2));
  ASSERT_FLOAT_EQ(0.5609756097f, result->getValue<storage::hyrise_float_t>(5, 3));
}

TEST_F(MathTests, int_division_divideZero_test) {
  Division divInt;
  divInt.addInput(inputTable);
  divInt.addField(0);
  divInt.addField(3);
  divInt.setVType(IntegerType);
  divInt.setColName("DIV");

  ASSERT_THROW(divInt.execute(), std::runtime_error);
}

TEST_F(MathTests, float_division_divideZero_test) {
  Division divFloat;
  divFloat.addInput(inputTable);
  divFloat.addField(0);
  divFloat.addField(3);
  divFloat.setVType(FloatType);
  divFloat.setColName("DIV");

  ASSERT_THROW(divFloat.execute(), std::runtime_error);
}

TEST_F(MathTests, int_modulo_test) {
  Mod modInt;
  modInt.addInput(inputTable);
  modInt.addField(0);
  modInt.setVType(IntegerType);
  modInt.setDivisor(5.f);
  modInt.setColName("MOD");

  modInt.execute();
  const auto& result = modInt.getResultTable();

  ASSERT_TRUE(types::isCompatible(IntegerType, result->typeOfColumn(5)));
  ASSERT_EQ(3, result->getValue<storage::hyrise_int_t>(5, 0));
  ASSERT_EQ(0, result->getValue<storage::hyrise_int_t>(5, 1));
  ASSERT_EQ(0, result->getValue<storage::hyrise_int_t>(5, 2));
  ASSERT_EQ(-4, result->getValue<storage::hyrise_int_t>(5, 3));
}

TEST_F(MathTests, float_modulo_test) {
  Mod modFloat;
  modFloat.addInput(inputTable);
  modFloat.addField(4);
  modFloat.setVType(FloatType);
  modFloat.setDivisor(3.f);
  modFloat.setColName("MOD");

  modFloat.execute();
  const auto& result = modFloat.getResultTable();

  ASSERT_TRUE(types::isCompatible(FloatType, result->typeOfColumn(5)));
  ASSERT_FLOAT_EQ(2.2f, result->getValue<storage::hyrise_float_t>(5, 0));
  ASSERT_FLOAT_EQ(2.9f, result->getValue<storage::hyrise_float_t>(5, 1));
  ASSERT_NEAR(0.1f, result->getValue<storage::hyrise_float_t>(5, 2), 0.00001f);
  ASSERT_FLOAT_EQ(-1.1f, result->getValue<storage::hyrise_float_t>(5, 3));
}

TEST_F(MathTests, modulo_divideZero_test) {
  Mod modFloat;

  ASSERT_THROW(modFloat.setDivisor(0.f), std::runtime_error);
}
}
}
