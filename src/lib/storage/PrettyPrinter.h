// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_PRETTYPRINTER_H_
#define SRC_LIB_STORAGE_PRETTYPRINTER_H_

#include "helper/types.h"
#include "ftprinter/FTPrinter.h"
#include "helper/RangeIter.h"


class TableDiff;

class PrettyPrinter {
 public:
  template <typename T>
  static void special_print(T& input, std::ostream& outStream, const std::string tableName, const size_t& limit, const size_t& start);

  static void special_print(const Store* store, std::ostream& outStream, const std::string tableName, const size_t& limit, const size_t& start);

  static void print(const AbstractTable* const input,
                    std::ostream& outStream, const std::string tableName = "",
                    const size_t& limit = (size_t) -1, const size_t& start = 0);
  static void printDiff(const hyrise::storage::c_atable_ptr_t& input, const TableDiff& diff,
                        std::ostream& outStream, const std::string tableName = "",
                        const size_t& limit = (size_t) -1, const size_t& start = 0);
  static void writeTid(ftprinter::FTPrinter &tp, hyrise::tx::transaction_id_t tid);
  static void writeCid(ftprinter::FTPrinter &tp, hyrise::tx::transaction_cid_t cid);
};

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
    outStream << "(showing first " << limit << " rows)" << std::endl;
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

#endif // SRC_LIB_STORAGE_PRETTYPRINTER_H_
