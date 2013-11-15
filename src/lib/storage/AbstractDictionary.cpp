#include "storage/AbstractDictionary.h"

AbstractDictionary::~AbstractDictionary() = default;

namespace hyrise { namespace storage {

AbstractDictionaryFactory::~AbstractDictionaryFactory() = default;

std::size_t AbstractDictionaryFactory::size() const { return 0; }

}}
