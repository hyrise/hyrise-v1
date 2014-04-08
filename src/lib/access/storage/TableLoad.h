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
  TableLoad(const Parameters& parameters = Parameters());
  virtual ~TableLoad();

  void executePlanOperation();

  const std::string vname();
  void setTableName(const std::string& tablename);
  void setFileName(const std::string& filename);
  void setPath(const std::string& path);
  void setHeaderFileName(const std::string& filename);
  void setHeaderString(const std::string& header);
  void setUnsafe(const bool unsafe);
  void setRaw(const bool raw);
  void setDelimiter(const std::string& d);

 private:
  Parameters _parameters;
};
}
}

#endif  // SRC_LIB_ACCESS_TABLELOAD_H_
