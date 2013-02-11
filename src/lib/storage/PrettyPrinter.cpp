// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "storage/PrettyPrinter.h"

#include <ostream>
#include <sstream>
#include <string>

#include "helper/types.h"
#include "helper/RangeIter.h"
#include "storage/AbstractTable.h"
#include "storage/RawTable.h"
#include "storage/storage_types.h"
#include "storage/TableDiff.h"
#include "ftprinter/FTPrinter.h"

using namespace hyrise;

template <typename T>
std::string generateValue(T& input, const size_t& column, const size_t& row) {
  std::stringstream buffer;

  // Add a basic hack to make valid_to columns look decent
  if ((input->nameOfColumn(column) == insertonly::VALID_TO_COL_ID) &&
      ((input->template getValue<hyrise_int_t>(column, row)) == insertonly::VISIBLE)) {
    buffer << "inf";
  } else {
    buffer << input->printValue(column, row);
  }

  try {
    ValueId valueId = input->getValueId(column, row);
    buffer << "(" << valueId.valueId << "@" << std::dec << (unsigned) valueId.table << ")";
  } catch (const std::runtime_error&) {}
  return buffer.str();
}

template <typename T>
void special_print(T& input, const std::string tableName, std::ostream& outStream, const size_t& limit, const size_t& start) {
  ftprinter::FTPrinter tp(tableName, outStream);
  tp.addColumn("#rowid", 6);
  const size_t columns = input->columnCount();
  for (size_t column_index = 0; column_index < columns; ++column_index) {
    // Auto adjusting widths means iterating over the table twice, but we use it for
    // debugging purposes only, anyways, so we'll go with beauty of output here
    auto name = input->nameOfColumn(column_index);
    size_t width = std::accumulate(RangeIter(0), RangeIter(input->size()),
                                   // minimum width is 4
                                   name.size() > 4 ? name.size() : 4,
                                   [&] (size_t max, const size_t& row) -> size_t {
                                     size_t sz = generateValue(input, column_index, row).size();
                                     return sz > max ? sz : max;
                                   });
    tp.addColumn(name, width);
  }

  if (limit < (size_t) - 1) {
    outStream << "(showing first " << limit << " rows)" << std::endl;
  }

  tp.printTableName();
  tp.printHeader();
  for (size_t row = start; row < input->size() && row < limit; ++row) {
    tp << row;
    for (size_t column = 0; column < columns; ++column) {
      tp << generateValue(input, column, row);
    }
  }
  tp.printFooter();
}

void PrettyPrinter::printDiff(const storage::c_atable_ptr_t& input, const TableDiff& diff,
                              const std::string tableName, std::ostream& outStream,
			      const size_t& limit, const size_t& start) {
  ftprinter::FTPrinter tp(tableName, outStream);
  tp.addColumn("#rowid", 6);
  const size_t columns = input->columnCount();

  for (size_t column_index = 0; column_index < columns; ++column_index) {
    // Auto adjusting widths means iterating over the table twice, but we use it for
    // debugging purposes only, anyways, so we'll go with beauty of output here
    auto name = input->nameOfColumn(column_index);
    size_t width = std::accumulate(RangeIter(0), RangeIter(input->size()),
                                   // minimum width is 4
                                   name.size() > 4 ? name.size() : 4,
                                   [&] (size_t max, const size_t& row) -> size_t {
                                     size_t sz = generateValue(input, column_index, row).size();
                                     return sz > max ? sz : max;
                                   });
    ftprinter::PrintFormat format = ftprinter::format::basic;
    if (diff.fields[column_index] == TableDiff::FieldWrong)
      format = ftprinter::format::red;
    else if(diff.fields[column_index] == TableDiff::FieldWrongType)
      format = ftprinter::format::magenta;

    tp.addColumn(name, width, format);
  }
  outStream << std::endl;

  if (limit < (size_t) - 1) {
    outStream << "(showing first " << limit << " rows)" << std::endl;
  }

  auto iWrong = diff.wrongRows.begin();
  auto iFalsePos = diff.falsePositionRows.begin();

  while (iWrong != diff.wrongRows.end() && *iWrong < start) iWrong++;
  while (iFalsePos != diff.falsePositionRows.end() && (*iFalsePos).first < start) iFalsePos++;

  tp.printTableName();
  tp.printHeader();

  for (size_t row = start; row < input->size() && row < limit; ++row) {
    tp << row;

    if (iWrong != diff.wrongRows.end() && *iWrong == row) {
      tp << ftprinter::format::red;
      iWrong++;
    }
    if (iFalsePos != diff.falsePositionRows.end() && (*iFalsePos).first == row) {
      tp << ftprinter::format::yellow;
      iFalsePos++;
    }

    for (field_t column = 0; column < columns; ++column) {
      tp << generateValue(input, column, row);
    }
  }
  tp.printFooter();
};

void PrettyPrinter::print(const AbstractTable* const input, const std::string tableName, std::ostream& outStream, const size_t& limit, const size_t& start) {
  const RawTable<>* r = dynamic_cast<const RawTable<>*>(input);
  if (r) {
    special_print(r, tableName, outStream, limit, start);
  } else {
    special_print(input, tableName, outStream, limit, start);
  }
}
