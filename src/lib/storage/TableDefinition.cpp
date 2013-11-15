#include "storage/TableDefinition.h"

namespace hyrise { namespace storage {

ColumnDefinition::ColumnDefinition(DataType type,
                                   std::string name,
                                   std::unique_ptr<AbstractDictionaryFactory> df) :
    _type(type), _name(name), _dictionary_factory(std::move(df)) {}


TableDefinition::TableDefinition(std::size_t size,
                                 std::unique_ptr<AbstractAttributeVectorFactory> af,
                                 std::vector<ColumnDefinition> cols) :
    _size(size), _attribute_vector_factory(std::move(af)), _columns(std::move(cols)) {}

}}
