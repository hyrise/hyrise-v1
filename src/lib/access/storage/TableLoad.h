// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_TABLELOAD_H_
#define SRC_LIB_ACCESS_TABLELOAD_H_

#include "access/system/PlanOperation.h"
#include "helper/serialization.h"

namespace hyrise {
namespace access {

class LoadTests_simple_load_op_Test;
class LoadTests_simple_unloadall_op_Test;

class TableLoad : public PlanOperation {
  friend class LoadTests_simple_load_op_Test;
  friend class LoadTests_simple_unloadall_op_Test;

 public:
  struct Parameters {
    std::string type, table, filename;
    std::optional<std::string> header, header_string, delimiter, path;
    std::optional<bool> unsafe, raw;

    SERIALIZE(type, table, filename, header, header_string, delimiter, path, unsafe, raw)
  };

 public:
  TableLoad();
  TableLoad(const Parameters& parameters);
  virtual ~TableLoad();

  void executePlanOperation();

  const std::string vname();
  void setTableName(const std::string& tablename);
  void setFileName(const std::string& filename);
  void setPath(const std::string& path);
  void setHeaderFileName(const std::string& filename);
  void setHeaderString(const std::string& header);
  void setBinary(const bool binary);
  void setUnsafe(const bool unsafe);
  void setRaw(const bool raw);
  void setDelimiter(const std::string& d);
  void setNonvolatile(const bool nonvolatile);

 private:
  std::string _table_name;
  std::string _file_name;
  std::optional<std::string> _header_file_name;
  std::optional<std::string> _header_string;
  std::optional<std::string> _delimiter;
  std::optional<std::string> _path;
  std::optional<bool> _unsafe;
  std::optional<bool> _raw;
  bool _nonvolatile;
  bool _binary;
};
}
}

#endif  // SRC_LIB_ACCESS_TABLELOAD_H_
