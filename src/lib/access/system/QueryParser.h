// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_QUERYPARSER_H_
#define SRC_LIB_ACCESS_QUERYPARSER_H_

#include <map>
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include <mutex>

#include <json.h>

#include "access/system/BasicParser.h"

const std::string autojsonReferenceTableId = "-1";

namespace hyrise {
namespace taskscheduler {
class Task;
} // namespace taskscheduler

namespace access {

class PlanOperation;


class QueryParserException : public std::runtime_error {
 public:
  explicit QueryParserException(const std::string &what): std::runtime_error(what)
  {}
};

struct AbstractQueryParserFactory {
  virtual std::shared_ptr<PlanOperation> parse(const Json::Value& data) = 0;

  virtual ~AbstractQueryParserFactory() {}
};

struct parse_construct {};
struct default_construct {};

template <typename T, typename parse_construction>
struct QueryParserFactory;

template<typename T>
struct QueryParserFactory<T, parse_construct> : public AbstractQueryParserFactory {

  virtual std::shared_ptr<PlanOperation> parse(const Json::Value& data) {
    return T::parse(data);
  }
};

template<typename T>
struct QueryParserFactory<T, default_construct> : public AbstractQueryParserFactory {
  virtual std::shared_ptr<PlanOperation> parse(const Json::Value& data) {
    typedef ::BasicParser<T> parser_t;
    return parser_t::parse(data);
  }
};

/*
 * The Query Parser parses a given Json Value to create a plan operation
 *
 */
class QueryParser {
  typedef std::map< std::string, AbstractQueryParserFactory * > factory_map_t;
  typedef std::map< Json::Value, std::shared_ptr<taskscheduler::Task> > task_map_t;

  factory_map_t _factory;
  QueryParser();

  /*  Builds tasks based on query's specifications and collects them in
      tasks and task_map for further processing. */
  void buildTasks(
      const Json::Value &query,
      std::vector<std::shared_ptr<taskscheduler::Task> > &tasks,
      task_map_t &task_map) const;

  /*  Defines operations input based on their types.  */
  void setInputs(
      std::shared_ptr<PlanOperation> planOperation,
      const Json::Value &planOperationSpec) const;

  //  Returns PAPI event name, if specified.
  std::string getPapiEventName(const Json::Value &query) const;
  //  Returns session id, if specified.
  int getSessionId(const Json::Value &query) const;

  //  Builds tasks' dependencies based on task map.
  void setDependencies(
      const Json::Value &query,
      task_map_t &task_map) const;

  //  Output of task without successor is query's result.
  std::shared_ptr<taskscheduler::Task> getResultTask(
      const task_map_t &task_map) const;

 public:
  ~QueryParser();

  template<typename T>
  static bool registerPlanOperation() {
    QueryParser::instance()._factory[T::name()] = new QueryParserFactory<T, parse_construct>();
    return true;
  }

  template<typename T>
  static bool registerPlanOperation(const std::string& name) {
    QueryParser::instance()._factory[name] = new QueryParserFactory<T, parse_construct>();
    return true;
  }

  template<typename T>
  static bool registerTrivialPlanOperation(const std::string& name) {
    QueryParser::instance()._factory[name] = new QueryParserFactory<T, default_construct>();
    return true;
  }
  
  std::shared_ptr<PlanOperation> parse(std::string name, const Json::Value& d);
  
  static QueryParser &instance();

  std::vector<std::string> getOperationNames() const;

  /*  Main method. Builds and returns executable PlanOperation tasks based on the
      query's specifications and constructs their dependency graph. The task
      delivering the final result will be determined, too.   */
  std::vector<std::shared_ptr<taskscheduler::Task> > deserialize(
      const Json::Value& query,
      std::shared_ptr<taskscheduler::Task> *result) const;
};

}}

#endif  // SRC_LIB_ACCESS_QUERYPARSER_H_
