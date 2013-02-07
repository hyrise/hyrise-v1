#ifndef SRC_LIB_IO_VECTORLOADER_H_
#define SRC_LIB_IO_VECTORLOADER_H_

#include "io/AbstractLoader.h"

class VectorInput : public AbstractInput {
 public:
  typedef std::vector<std::uint64_t> value_vec_t;
  typedef std::vector<value_vec_t> value_vectors_t;
  VectorInput(value_vectors_t vectors);
  VectorInput *clone() const;
  std::shared_ptr<AbstractTable> load(std::shared_ptr<AbstractTable>,
                                      const compound_metadata_list *,
                                      const Loader::params &args);
 private:
  const value_vectors_t _vectors;
};

#endif
