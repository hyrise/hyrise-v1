// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#ifdef WITH_V8

#include "helper.h"
#include <io/shortcuts.h>
#include <helper/Settings.h>
#include <access/ScriptOperation.h>
#include <testing/TableEqualityTest.h>

namespace hyrise {
namespace access {

class ScriptOperationTests : public AccessTest {
 public:
  ScriptOperationTests() {
    auto s = Settings::getInstance();
    s->setScriptPath("./test/script");
    tab = Loader::shortcuts::load("test/index_test.tbl");
  }

  hyrise::storage::c_atable_ptr_t tab;
};

TEST_F(ScriptOperationTests, pointer_calculator) {
  ScriptOperation op;
  op.addInput(tab);
  op.setScriptName("hello");
  op.execute();

  auto reference = Loader::shortcuts::load("test/reference/ops_script_pc_ref.tbl");
  auto result = op.getResultTable();
  EXPECT_RELATION_EQ(result, reference);
}

TEST_F(ScriptOperationTests, copy_structure_modifiable) {
  ScriptOperation op;
  op.addInput(tab);
  op.setScriptName("copy_structure");
  op.execute();

  auto reference = Loader::shortcuts::load("test/reference/ops_script_mutable_ref.tbl");
  auto result = op.getResultTable();
  EXPECT_RELATION_EQ(result, reference);
}

TEST_F(ScriptOperationTests, cant_set_values_on_input_table) {
  ScriptOperation op;
  op.addInput(tab);
  op.setScriptName("no_set_values");
  EXPECT_ANY_THROW(op.execute());
}

TEST_F(ScriptOperationTests, include_working) {
  ScriptOperation op;
  op.addInput(tab);
  op.setScriptName("test_with_include");
  op.execute();
}

TEST_F(ScriptOperationTests, include_working_wrong_file) {
  ScriptOperation op;
  op.addInput(tab);
  op.setScriptName("test_with_include_bad");
  EXPECT_ANY_THROW(op.execute());
}

TEST_F(ScriptOperationTests, build_table) {
  ScriptOperation op;
  op.addInput(tab);
  op.setScriptName("build_table");
  op.execute();

  auto reference = Loader::shortcuts::load("test/tables/companies.tbl");
  auto result = op.getResultTable();
  EXPECT_RELATION_EQ(result, reference);
}

TEST_F(ScriptOperationTests, build_table_wrong) {
  ScriptOperation op;
  op.addInput(tab);
  op.setScriptName("build_table_bad");
  EXPECT_ANY_THROW(op.execute());
}

TEST_F(ScriptOperationTests, build_vertical_table) {
  ScriptOperation op;
  op.addInput(tab);
  op.setScriptName("build_vtab");
  op.execute();

  auto reference = Loader::shortcuts::load("test/tables/companies.tbl");
  auto result = op.getResultTable();
  EXPECT_RELATION_EQ(result, reference);
}

TEST_F(ScriptOperationTests, get_attribute_vectors_scan) {
  ScriptOperation op;
  op.addInput(tab);
  op.setScriptName("get_attribute_vectors_scan");
  op.execute();

  auto reference = Loader::shortcuts::load("test/reference/ops_script_pc_ref.tbl");
  auto result = op.getResultTable();
  EXPECT_RELATION_EQ(result, reference);
}

TEST_F(ScriptOperationTests, get_attribute_vectors_scan_range) {
  ScriptOperation op;
  op.addInput(tab);
  op.setScriptName("get_attribute_vectors_scan_range");
  op.execute();

  auto reference = Loader::shortcuts::load("test/reference/ops_script_pc_ref.tbl");
  auto result = op.getResultTable();
  EXPECT_RELATION_EQ(result, reference);
}
}
}

#endif
