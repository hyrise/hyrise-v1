#include "io/EmptyLoader.h"

std::shared_ptr<AbstractTable> EmptyInput::load(std::shared_ptr<AbstractTable> table, const compound_metadata_list *t, const Loader::params &args) {
  return table;
}

EmptyInput *EmptyInput::clone() const {
  return new EmptyInput(*this);
}

compound_metadata_list *EmptyHeader::load(const Loader::params &args) {
  compound_metadata_list *l = new compound_metadata_list();
  return l;
}

EmptyHeader *EmptyHeader::clone() const {
  return new EmptyHeader(*this);
}
