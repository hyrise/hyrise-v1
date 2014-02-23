// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_TABLELOAD_H_
#define SRC_LIB_ACCESS_TABLELOAD_H_

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

class LoadTests_simple_load_op_Test;
class LoadTests_simple_unloadall_op_Test;

class TableLoad : public PlanOperation {
  friend class LoadTests_simple_load_op_Test;
  friend class LoadTests_simple_unloadall_op_Test;

public:
  TableLoad();
  virtual ~TableLoad();

  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
  const std::string vname();
  void setTableName(const std::string &tablename);
  void setFileName(const std::string &filename);
  void setHeaderFileName(const std::string &filename);
  void setHeaderString(const std::string &header);
  void setBinary(const bool binary);
  void setUnsafe(const bool unsafe);
  void setRaw(const bool raw);
  void setDelimiter(const std::string &d);

private:
  std::string _table_name;
  std::string _header_file_name;
  std::string _file_name;
  std::string _header_string;
  std::string _delimiter;
  bool _hasDelimiter;
  bool _binary;
  bool _unsafe;
  bool _raw;
};

}
}

#endif  // SRC_LIB_ACCESS_TABLELOAD_H_
