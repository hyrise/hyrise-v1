// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "TableEqualityTest.h"

#include <storage/AbstractTable.h>
#include <storage/TableDiff.h>
#include <storage/PrettyPrinter.h>
#include <storage/storage_types_helper.h>
namespace hyrise {

using namespace storage;

std::string schemaErrors(TableDiff diff, const char* relationName, tblptr table) {
  std::vector<std::string> fieldError, fieldTypeError;
  std::stringstream buf;

  for (field_t i = 0; i < diff.fields.size(); ++i) {
    if (diff.fields[i] != TableDiff::FieldCorrect) {
      if (diff.fields[i] == TableDiff::FieldWrongType) {
        hyrise::types::type_t type = data_type_to_string(table->typeOfColumn(i));
        fieldTypeError.push_back(table->nameOfColumn(i) + "(" + type + ")");
      }
      else
        fieldError.push_back(table->nameOfColumn(i));
    }
  }

  if (! (fieldError.empty() && fieldTypeError.empty()) ) {
    buf << "Error in \"" << relationName << "\"s relation schema:" << std::endl;
    if (!fieldError.empty()) {
      buf << "mismatched fields: ";
      for (size_t i = 0; i < fieldError.size()-1; i++)
        buf << fieldError[i] << ", ";
      buf << fieldError.back() << std::endl;
    }
    if (!fieldTypeError.empty()) {
      buf << "field type mismatches: ";
      for (size_t i = 0; i < fieldTypeError.size()-1; i++)
        buf << fieldTypeError[i] << ", ";
      buf << fieldTypeError.back() << std::endl;
    }
  }

  return buf.str();
}

std::string rowErrors(TableDiff diff, const char* baseRelationName, const char* otherRelationName) {
  if (diff.wrongRows.empty())
    return "";

  std::stringstream buf;

  buf << "rows in \"" << baseRelationName << "\" that are not in \""
            << otherRelationName << "\": ";
  for (size_t i = 0; i < diff.wrongRows.size()-1; ++i)
    buf << diff.wrongRows[i] << ", ";
  buf << diff.wrongRows.back() << std::endl;

  return buf.str();
}

std::string rowPositionErrors(TableDiff diff, const char* baseRelationName, const char* otherRelationName) {
  if (diff.falsePositionRows.empty())
    return "";

  std::stringstream buf;

  buf << "rows in \"" << baseRelationName << "\" that are not in the same place as in\""
            << otherRelationName << "\": ";
  for (auto i = diff.falsePositionRows.begin(); i != diff.falsePositionRows.end(); ++i)
    buf << (*i).first << " (" << (*i).second << "), ";
  //TODO: don't let the list end with ','
  //buf << (*diff.falsePositionRows.end()).first << "(" << (*diff.falsePositionRows.end()).second << ")" << std::endl;

  return buf.str();
}


::testing::AssertionResult RelationEquals(const char* left_exp,
    const char* right_exp,
    tblptr left, tblptr right, bool schema_only) {
  auto resultL2R = TableDiff::diffTables(left.get(),  right.get(), schema_only);
  auto resultR2L = TableDiff::diffTables(right.get(), left.get(), schema_only);

  if (resultL2R.equal() && resultR2L.equal()) {
    return ::testing::AssertionSuccess();
  }

  std::stringstream buf;

  PrettyPrinter::printDiff(left, resultL2R, buf, left_exp);
  buf << std::endl;
  PrettyPrinter::printDiff(right, resultR2L, buf, right_exp);

  buf << "\"" << left_exp << "\""
      << "is not an equal relation to"
      << "\"" << right_exp << "\":" << std::endl
      << schemaErrors(resultL2R, left_exp, left)
      << schemaErrors(resultR2L, right_exp, right);
  if (!schema_only) {
    buf << rowErrors(resultL2R, left_exp, right_exp)
        << rowErrors(resultR2L, right_exp, left_exp);
  }

  return ::testing::AssertionFailure() << buf.str();
}

::testing::AssertionResult RelationNotEquals(const char* left_exp,
    const char* right_exp,
    tblptr left, tblptr right, bool schema_only) {
  auto resultL2R = TableDiff::diffTables(left.get(),  right.get(), schema_only);
  auto resultR2L = TableDiff::diffTables(right.get(), left.get(), schema_only);

  if (resultL2R.equal() && resultR2L.equal())
    return ::testing::AssertionFailure() << left_exp << " and " << right_exp
           << " are not expected to be equal";

  return ::testing::AssertionSuccess();
}

::testing::AssertionResult SchemaEquals(const char *left_exp,
                                          const char *right_exp,
                                          tblptr left,
                                          tblptr right) {
   return RelationEquals(left_exp, right_exp, left, right, true);
}


::testing::AssertionResult SortedRelationEquals(const char* left_exp,
    const char* right_exp,
    tblptr left,
    tblptr right) {
  auto resultL2R = TableDiff::diffTables(left.get(),  right.get());
  auto resultR2L = TableDiff::diffTables(right.get(), left.get());

  if (resultL2R.equal() && resultR2L.equal()) {
    return ::testing::AssertionSuccess();
  }

  std::stringstream buf;

  PrettyPrinter::printDiff(left, resultL2R, buf, left_exp);
  buf << std::endl;
  PrettyPrinter::printDiff(right, resultR2L, buf, right_exp);

  buf << "\"" << left_exp << "\""
      << "is not an equal sorted relation to"
      << "\"" << right_exp << "\"" << std::endl
      << schemaErrors(resultL2R, left_exp, left)
      << schemaErrors(resultR2L, right_exp, right)
      << rowErrors(resultL2R, left_exp, right_exp)
      << rowErrors(resultR2L, right_exp, left_exp)
      << rowPositionErrors(resultL2R, left_exp, right_exp);

  return ::testing::AssertionFailure() << buf.str();
}

::testing::AssertionResult SortedRelationNotEquals(const char* left_exp,
    const char* right_exp,
    tblptr left, tblptr right) {
  auto resultL2R = TableDiff::diffTables(left.get(),  right.get());
  auto resultR2L = TableDiff::diffTables(right.get(), left.get());

  if (resultL2R.equalSorted() && resultR2L.equalSorted())
    return ::testing::AssertionFailure() << left_exp << " and " << right_exp
           << " are not expected to be equal";

  return ::testing::AssertionSuccess();
}

} // namespace hyrise

