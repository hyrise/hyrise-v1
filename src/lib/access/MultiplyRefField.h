// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCES_MULTIPLYREFFIELD_H
#define SRC_LIB_ACCES_MULTIPLYREFFIELD_H

#include "access/system/PlanOperation.h"

#include "storage/ColumnMetadata.h"
#include "storage/MutableVerticalTable.h"
#include "storage/PointerCalculator.h"

namespace hyrise {
namespace access {

/// This class implements a horizontal multiply operation that allows to specify
/// a reference field and a value field so that for all rows of the second
/// input table are multiplied by the value field
///
/// The input to the plan operation are two tables: the first input
/// specifies the source table and the second table the table to multiply.
/// The first input field defines the reference field while the second input
/// field defines the value field to use as a mulitplier. The format of the
/// target field depends on the value field.
class MultiplyRefField : public PlanOperation {
public:
  void executePlanOperation();
	/// Example JSON for the plan operation, fields defines the reference and value
	/// the value field
  /// "multiply" : {
  ///   "type" : "MultiplyRefField",
  ///   "fields" : ["ref", "value"],
  /// }
  static std::shared_ptr<PlanOperation> parse(Json::Value &data);
  const std::string vname();

private:
	template<typename T, DataType D>
	void executeMultiply();
};

template<typename T, DataType D>
void MultiplyRefField::executeMultiply() {
	const auto ref = _field_definition[0];
	const auto val = _field_definition[1];

	const auto stop = getInputTable()->size();
	const auto stopInner = getInputTable(1)->size();

	const auto &itab = getInputTable();
	const auto &mulTab = getInputTable(1);

	std::vector<const ColumnMetadata*> meta {new ColumnMetadata("value", D), new ColumnMetadata("pos", IntegerType)};
	auto result = std::make_shared<Table>(&meta, nullptr, stop * stopInner, false, false);
	result->resize(stop * stopInner);

	// Pos list for matching Rows
	auto pos = new std::vector<size_t>;

	// Nested Loop for Multiplication
	for(size_t outer=0; outer < stop; ++outer) {
		const auto tmp = itab->getValue<T>(val, outer);
		const auto col = itab->getValue<hyrise_int_t>(ref, outer);

		for(size_t inner=0; inner < stopInner; ++inner) {
			pos->push_back(outer);
			result->setValue<T>(0, stopInner * outer + inner, mulTab->getValue<T>(col, inner) * tmp);
			result->setValue<hyrise_int_t>(1, stopInner * outer + inner, inner);
		}
	}

	auto left = PointerCalculator::create(getInputTable(), pos);

	std::vector<storage::atable_ptr_t> vc({left, result});
	storage::atable_ptr_t tt = std::make_shared<storage::MutableVerticalTable>(vc);
	addResult(tt);
}

}
}

#endif // SRC_LIB_ACCES_MULTIPLYREFFIELD_H
