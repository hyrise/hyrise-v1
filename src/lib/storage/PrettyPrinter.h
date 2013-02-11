// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_PRETTYPRINTER_H_
#define SRC_LIB_STORAGE_PRETTYPRINTER_H_

#include "helper/types.h"

class TableDiff;

class PrettyPrinter {
 public:
  static void print(const AbstractTable* const input,
                    const std::string tableName, std::ostream& outStream,
                    const size_t& limit = (size_t) -1, const size_t& start = 0);
  static void printDiff(const hyrise::storage::c_atable_ptr_t& input, const TableDiff& diff,
                        const std::string tableName, std::ostream& outStream,
                        const size_t& limit = (size_t) -1, const size_t& start = 0);
};

#endif // SRC_LIB_STORAGE_PRETTYPRINTER_H_
