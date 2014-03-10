#include "access/expressions/ExpressionRegistration.h"

namespace hyrise {
namespace access {

// std::map<std::string, expression_factory_ptr_t> Expressions::_registrations;

Expressions& Expressions::getInstance() {
  static Expressions e;
  return e;
}

bool Expressions::addRegistration(const std::string& callsign, expression_factory_ptr_t ptr) {
  _registrations.insert(std::make_pair(callsign, std::move(ptr)));
  return true;
}

expression_uptr_t Expressions::createRegistered(const std::string& callsign, const Json::Value& value) const {
  return _registrations.at(callsign)->create(value);
}
}
}
