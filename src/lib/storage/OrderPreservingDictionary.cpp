#include "OrderPreservingDictionary.h"

namespace hyrise { namespace storage {

OrderPreservingDictionaryIteratorString::OrderPreservingDictionaryIteratorString(dictionary_type *values) : _data(values){
}

OrderPreservingDictionaryIteratorString::OrderPreservingDictionaryIteratorString(dictionary_type *values, size_t index) : _data(values), _index(index) {
}

void OrderPreservingDictionaryIteratorString::increment() {
  ++_index;
}

bool OrderPreservingDictionaryIteratorString::equal(const std::shared_ptr<BaseIterator<std::string>>& other) const {
  const auto& op = std::dynamic_pointer_cast<base_type>(other);
  return _data == op->_data && _index == op->_index;
}

std::string OrderPreservingDictionaryIteratorString::dereference() {
  return _data->get_s(_index);
}

value_id_t OrderPreservingDictionaryIteratorString::getValueId() const {
  return _index;
}


}}
