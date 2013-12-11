// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "storage/PrettyPrinter.h"

#include <numeric>
#include <iostream>
#include <sstream>
#include <string>

#include "helper/types.h"
#include "helper/RangeIter.h"
#include "storage/AbstractTable.h"
#include "storage/Store.h"
#include "storage/RawTable.h"
#include "storage/storage_types.h"
#include "storage/TableDiff.h"

namespace hyrise { namespace storage {

template <typename T>
std::string generateValue(T& input, const size_t column, const size_t row) {
  std::stringstream buffer;
  buffer << input->printValue(column, row);
  try {
    ValueId valueId = input->getValueId(column, row);
    buffer << "(" << valueId.valueId << "@" << std::dec << (unsigned) valueId.table << ")";
  } catch (const std::runtime_error&) {}
  return buffer.str();
}

template <typename T>
void PrettyPrinter::special_print(T& input, std::ostream& outStream, const std::string tableName, const size_t& limit, const size_t& start) {
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
    outStream << "(showing first " << limit << " of " << input->size() << " rows)" << std::endl;
  }

  if (tableName.size() > 0)
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


void PrettyPrinter::special_print(const Store* store, std::ostream& outStream, const std::string tableName, const size_t& limit, const size_t& start) {
  ftprinter::FTPrinter tp(tableName, outStream);
  tp.addColumn("#rowid", 6);
  const size_t columns = store->columnCount();
  for (size_t column_index = 0; column_index < columns; ++column_index) {
    // Auto adjusting widths means iterating over the table twice, but we use it for
    // debugging purposes only, anyways, so we'll go with beauty of output here
    auto name = store->nameOfColumn(column_index);
    size_t width = std::accumulate(RangeIter(0), RangeIter(store->size()),
                                   // minimum width is 4
                                   name.size() > 4 ? name.size() : 4,
                                   [&] (size_t max, const size_t row) -> size_t {
                                     size_t sz = generateValue(store, column_index, row).size();
                                     return sz > max ? sz : max;
                                   });
    tp.addColumn(name, width);
  }
  tp.addColumn("$tid",6);
  tp.addColumn("$cidbeg",6);
  tp.addColumn("$cidend",6);

  if (limit < (size_t) - 1) {
    outStream << "(showing first " << limit << " rows)" << std::endl;
  }

  if (tableName.size() > 0)
    tp.printTableName();

  tp.printHeader();

  for (size_t row = start; row < store->size() && row < limit; ++row) {
    tp << row;
    for (size_t column = 0; column < columns; ++column) {
      tp << generateValue(store, column, row);
    }
    writeTid(tp, store->_tidVector[row]);
    writeCid(tp, store->_cidBeginVector[row]);
    writeCid(tp, store->_cidEndVector[row]);
  }
  tp.printFooter();
}

void PrettyPrinter::printDiff(const c_atable_ptr_t& input, const TableDiff& diff,
                              std::ostream& outStream, const std::string tableName,
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

  if (tableName.size() > 0)
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

void PrettyPrinter::print(const AbstractTable* const input, std::ostream& outStream, const std::string tableName, const size_t& limit, const size_t& start) {
  auto* r = dynamic_cast<const RawTable*>(input);
  if (r) {
    special_print(r, outStream, tableName, limit, start);
  } else {
    const Store* s = dynamic_cast<const Store*>(input);
    if (s)
      special_print(s, outStream, tableName, limit, start);
    else
      special_print(input, outStream, tableName, limit, start);
  }
}

void PrettyPrinter::writeTid(ftprinter::FTPrinter &tp, tx::transaction_id_t tid) {
  switch(tid) {
    case tx::MERGE_TID:
      tp << "MERGE";
      break;
    case tx::START_TID:
      tp << "START";
      break;
    case tx::MAX_TID:
      tp << "MAX";
      break;
    case tx::UNKNOWN:
      tp << "NONE";
      break;
    default:
      tp << tid;
      break;
  }
}

void PrettyPrinter::writeCid(ftprinter::FTPrinter &tp, tx::transaction_cid_t cid) {
  switch(cid) {
    case tx::UNKNOWN_CID:
      tp << "NONE";
      break;
    case tx::INF_CID:
      tp << "INF";
      break;
    default:
      tp << cid;
      break;
  }
}

}}
