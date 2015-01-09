// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "transformation_helper.h"
#include <algorithm>

using namespace hsql;

namespace hyrise {
namespace access {
namespace sql {



template<typename _T>
std::vector<_T> combineVectors(std::vector<_T> v1, std::vector<_T> v2) {
  std::vector<_T> v;
  v.reserve(v1.size() + v2.size());
  v.insert(v.end(), v1.begin(), v1.end());
  v.insert(v.end(), v2.begin(), v2.end());
  return v;
}

// Explicit instantiation
template std::vector<std::string> combineVectors(std::vector<std::string> v1, std::vector<std::string> v2);



int identifyTableForColumnRef(Expr* col, TransformationResult t1, TransformationResult t2) {
  if (col->hasTable()) {
    if (t1.hasName(col->table)) return 0;
    if (t2.hasName(col->table)) return 1;
    
    SQLStatementTransformer::throwError("Can't find table referenced in column", col->table);
  } 

  bool inT1 = t1.containsField(col->name);
  bool inT2 = t2.containsField(col->name);
  if (inT1 && inT2) return -1; // In both tables
  if (inT1) return 0;
  if (inT2) return 1;
  return -2; // In neither table
}




std::string buildFunctionRefColumnName(Expr* func_ref) {
  std::string column_name = std::string(func_ref->name) + "(" + std::string(func_ref->expr->name) + ")";
  return column_name;
}

std::string stringToUppercase(std::string str) {
  std::string copy(str);
  for_each(copy.begin(), copy.end(), [](char& in){ in = ::toupper(in); });
  return copy;
}

} // namespace sql
} // namespace access
} // namespace hyrise