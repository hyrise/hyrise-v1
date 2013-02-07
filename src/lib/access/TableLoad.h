#ifndef SRC_LIB_ACCESS_TABLELOAD_H_
#define SRC_LIB_ACCESS_TABLELOAD_H_

#include <access/PlanOperation.h>

namespace hyrise {
namespace access {
class LoadTests_simple_load_op_Test;
class LoadTests_simple_unloadall_op_Test;
}
}

class TableLoad : public _PlanOperation {
  friend class hyrise::access::LoadTests_simple_load_op_Test;
  friend class hyrise::access::LoadTests_simple_unloadall_op_Test;
 public:
  TableLoad(): _hasDelimiter(false), _binary(false), _unsafe(false) {
  }

  virtual ~TableLoad() {
  }

  void executePlanOperation();

  void setTableName(std::string tablename) {
    _table_name = tablename;
  }
  void setFileName(std::string filename) {
    _file_name = filename;
  }
  void setHeaderFileName(std::string filename) {
    _header_file_name = filename;
  }
  void setHeaderString(std::string header) {
    _header_string = header;
  }
  void setBinary(bool binary) {
    _binary = binary;
  }
  void setUnsafe(bool unsafe) {
    _unsafe = unsafe;
  }

  void setRaw(bool raw) {
    _raw = raw;
  }

  void setDelimiter(std::string d) {
    _delimiter = d;
    _hasDelimiter = true;
  }

  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  static bool is_registered;
  static std::string name() {
    return "TableLoad";
  }

  const std::string vname() {
    return "TableLoad";
  }

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

#endif  // SRC_LIB_ACCESS_TABLELOAD_H_

