#include "access/AdhocSelection.h"

#include "storage/meta_storage.h"
#include "storage/AbstractTable.h"
#include "storage/PointerCalculator.h"

#include "mpParser.h"

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<AdhocSelection>("AdhocSelection");
// fix thread safety through early parser instantiation,
// internal parser module instantiation is not thread safe
mup::ParserX threadsafetyfix(mup::pckALL_NON_COMPLEX);
}

std::shared_ptr<PlanOperation> AdhocSelection::parse(const Json::Value& data) {
  return std::make_shared<AdhocSelection>(data["expression"].asString());
}

// We need to check the value range since muparserX currently only supports int32 for values
template <typename T>
void check_range(T, size_t, size_t) {}

template <>
void check_range(hyrise_int_t val, size_t column, size_t row) {
  if (!(std::numeric_limits<mup::int_type>::min() <= val) && (val <= std::numeric_limits<mup::int_type>::max())) {
    throw std::runtime_error("Int at col: " + std::to_string(column) + " row: " + std::to_string(row) + " (is '" +
                             std::to_string(val) + "') exceeds muparser int range");
  }
}

template <typename T>
mup::Value makeValue(T& val) {
  return {val};
}

template <>
mup::Value makeValue(hyrise_int_t& val) {
  return {static_cast<mup::int_type>(val)};
}

// Translate column/row position in table to MupValue
struct MupValueByType {
  std::size_t column;
  std::size_t row;
  storage::c_atable_ptr_t tbl;
  using value_type = mup::Value;

  template <typename T>
  mup::Value operator()() {
    auto&& value = tbl->getValue<T>(column, row);
    check_range(value, column, row);
    return makeValue<T>(value);
  }
};

// We parse the expression and then extract all identified free
// expression variables (which have to be columns of the incoming
// table) and use them to build up an intermediate structure of
// {datatype, offset, value} elements.  We then bind the expression
// variables against the value elements of the intermediate structure
// so that we can replace them for every single row and then evaluate
// the expression again.
storage::atable_ptr_t scanTableWithExpression(storage::c_atable_ptr_t tbl, const std::string& expression) {
  mup::ParserX parser(mup::pckALL_NON_COMPLEX);
  parser.SetExpr(expression);

  auto expression_vars = parser.GetExprVar();
  struct Field {
    DataType datatype;
    std::size_t column;
    mup::Value value;
  };

  std::vector<Field> fields;
  fields.reserve(expression_vars.size());
  for (const auto& expression_var : expression_vars) {
    const auto& var_name = expression_var.first;
    auto column = tbl->numberOfColumn(var_name);
    fields.emplace_back(Field{tbl->typeOfColumn(column), column, mup::Value{}});
    // Bind var_name to address of .value member we just added
    parser.DefineVar(var_name, mup::Variable(&fields.back().value));
  }

  pos_list_t positions;
  MupValueByType functor;
  functor.tbl = tbl;
  for (std::size_t row = 0, e = tbl->size(); row < e; ++row) {
    for (auto& field : fields) {
      functor.column = field.column;
      functor.row = row;
      field.value = storage::type_switch<hyrise_basic_types>()(field.datatype, functor);
    }

    mup::Value result = parser.Eval();
    if (result.GetBool()) {
      positions.push_back(row);
    }
  }
  return storage::PointerCalculator::create(tbl, std::move(positions));
}

void AdhocSelection::executePlanOperation() { addResult(scanTableWithExpression(input.getTable(0), _expression)); }
}
}
