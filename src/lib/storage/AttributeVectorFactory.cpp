#include "storage/AttributeVectorFactory.h"

#include "storage/FixedLengthVector.h"
#include "storage/ConcurrentFixedLengthVector.h"
#include "storage/BitCompressedVector.h"

namespace hyrise {
namespace storage {

baseattr_ptr_tr create_compressed_attribute_vector(size_t cols, size_t rows, std::vector<adict_ptr_t>& dicts) {
  std::vector<uint64_t> bits(dicts.size(), 0);
  std::transform(dicts.begin(), dicts.end(), bits.begin(), [](const adict_ptr_t& dict) {
    auto sz = dict->size();
    return sz == 0 ? 0ul : std::max(1ul, static_cast<uint64_t>(ceil(log(sz) / log(2.0))));
  });
  return std::make_shared<BitCompressedVector<value_id_t>>(cols, rows, bits);
}


/// Use this function to obtain an attribute vector
baseattr_ptr_tr create_attribute_vector(size_t cols,
                                        size_t rows,
                                        CONCURRENCY_FLAG concurrent,
                                        COMPRESSION_FLAG compressed,
                                        std::vector<adict_ptr_t>& dicts) {
  switch (concurrent) {
    case CONCURRENCY_FLAG::CONCURRENT:
      switch (compressed) {
        case COMPRESSION_FLAG::COMPRESSED:
          throw std::logic_error("Can't create compressed concurrent attribute vector");
        case COMPRESSION_FLAG::UNCOMPRESSED:
          return std::make_shared<ConcurrentFixedLengthVector<value_id_t>>(cols, rows);
      }
      break;
    case CONCURRENCY_FLAG::NOT_CONCURRENT:
      switch (compressed) {
        case COMPRESSION_FLAG::UNCOMPRESSED:
          return std::make_shared<FixedLengthVector<value_id_t>>(cols, rows);
        case COMPRESSION_FLAG::COMPRESSED:
          return create_compressed_attribute_vector(cols, rows, dicts);
      }
  }
  throw std::runtime_error("this should be unreachable");
}
}
}
