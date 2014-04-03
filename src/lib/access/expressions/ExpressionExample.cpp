// // Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
// #include "access/expressions/ExpressionExample.h"
// #include "access/expressions/ExpressionLibrary.h"
// #include "storage/AbstractTable.h"
// #include "helper/make_unique.h"

// namespace hyrise {
// namespace access {

// namespace {
// bool success = ExpressionLibrary::getInstance().add("asdd", &ExpressionExample::creator);
// }

// ExpressionExample::ExpressionExample(const size_t column, const hyrise_int_t value) : Expression() {
//   _column = column;
//   _value = value;
// }

// std::unique_ptr<Expression> ExpressionExample::creator(const Json::Value& data) {
//   return make_unique<ExpressionExample>(data["column"].asUInt(), data["value"].asInt());
// }

// void ExpressionExample::evaluateMain(pos_list_t* results) {
//   value_id_t valueId;

//   if (_mainVector == nullptr || _mainDictionary == nullptr)
//     throw std::runtime_error("mainVector or mainDictionary error");

//   valueId = _mainDictionary->getValueIdForValue(_value);
//   if (valueId == _mainVector->size())
//     return;

//   for (size_t currentRow = 0; currentRow < _mainVector->size(); ++currentRow) {
//     if (_mainVector->getRef(_column, currentRow) == valueId)
//       results->push_back(currentRow);
//   }
// }

// void ExpressionExample::evaluateDelta(pos_list_t* results) {
//   value_id_t valueId;
//   size_t deltaOffsetInTable = _mainVector->size();

//   if (_deltaVector == nullptr || _deltaDictionary == nullptr)
//     throw std::runtime_error("mainVector or mainDictionary error");

//   if (!_deltaDictionary->valueExists(_value))
//     return;

//   valueId = _deltaDictionary->getValueIdForValue(_value);

//   for (size_t currentRow = 0; currentRow < _deltaVector->size(); ++currentRow) {
//     if (_deltaVector->getRef(_column, currentRow) == valueId)
//       results->push_back(currentRow + deltaOffsetInTable);
//   }
// }

// void ExpressionExample::setup(const storage::c_atable_ptr_t& table) {
//   _table = table;
//   const auto& avs = table->getAttributeVectors(_column);
//   std::cout << avs.size() << std::endl;

//   _mainVector = std::dynamic_pointer_cast<storage::FixedLengthVector<value_id_t>>(avs.at(0).attribute_vector);
//   _mainDictionary =
//       std::dynamic_pointer_cast<storage::OrderPreservingDictionary<hyrise_int_t>>(table->dictionaryAt(_column));

//   if (deltaExists()) {
//     _deltaVector =
//         std::dynamic_pointer_cast<storage::ConcurrentFixedLengthVector<value_id_t>>(avs.at(1).attribute_vector);
//     _deltaDictionary = std::dynamic_pointer_cast<storage::ConcurrentUnorderedDictionary<hyrise_int_t>>(
//         table->dictionaryAt(_column, _mainVector->size()));
//   }
// }
// }
// }