#ifndef SRC_LIB_ACCESS_EXPRESSIONREGISTRATION_H_
#define SRC_LIB_ACCESS_EXPRESSIONREGISTRATION_H_

#include <memory>
#include <map>
#include <string>
#include "json.h"

#include "helper/types.h"
#include "helper/make_unique.h"
#include "helper/noncopyable.h"

#include "access/expressions/AbstractExpression.h"

namespace hyrise {
namespace access {

class AbstractExpressionFactory {
 public:
  virtual ~AbstractExpressionFactory() {}
  virtual expression_uptr_t create(const Json::Value& value) = 0;
};

typedef std::unique_ptr<AbstractExpressionFactory> expression_factory_ptr_t;

template <typename Expression>
class ExpressionFactory : public AbstractExpressionFactory {
 public:
  virtual expression_uptr_t create(const Json::Value& value) { return Expression::parse(value); }
};

/// Expression registration facility for custom expressions
class Expressions : noncopyable {
 public:
  static Expressions& getInstance();

  template <typename Expression>
  static bool add(const std::string& callsign) {
    return getInstance().addRegistration(callsign, make_unique<ExpressionFactory<Expression>>());
  }

  static expression_uptr_t parse(const std::string& callsign, const Json::Value& value) {
    return getInstance().createRegistered(callsign, value);
  }

 private:
  std::map<std::string, expression_factory_ptr_t> _registrations;

  bool addRegistration(const std::string& callsign, expression_factory_ptr_t ptr);
  expression_uptr_t createRegistered(const std::string& callsign, const Json::Value& value) const;
};
}
}
#endif
