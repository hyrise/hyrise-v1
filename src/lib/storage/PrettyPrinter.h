// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "helper/types.h"
#include "ftprinter/FTPrinter.h"
#include "helper/RangeIter.h"

namespace hyrise {
namespace storage {

class TableDiff;

class PrettyPrinter {
 public:
  template <typename T>
  static void special_print(T& input, std::ostream& outStream, const std::string tableName, const size_t& limit, const size_t& start);

  static void special_print(const Store* store, std::ostream& outStream, const std::string tableName, const size_t& limit, const size_t& start);

  static void print(const AbstractTable* const input,
                    std::ostream& outStream, const std::string tableName = "",
                    const size_t& limit = (size_t) -1, const size_t& start = 0);
  static void printDiff(const c_atable_ptr_t& input, const TableDiff& diff,
                        std::ostream& outStream, const std::string tableName = "",
                        const size_t& limit = (size_t) -1, const size_t& start = 0);
  static void writeTid(ftprinter::FTPrinter &tp, tx::transaction_id_t tid);
  static void writeCid(ftprinter::FTPrinter &tp, tx::transaction_cid_t cid);
};

}}

