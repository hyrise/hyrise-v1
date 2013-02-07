#ifndef SRC_LIB_IO_VALIDITYTABLEGENERATION_H_
#define SRC_LIB_IO_VALIDITYTABLEGENERATION_H_

#include "helper/types.h"

namespace hyrise {
namespace insertonly {

storage::atable_ptr_t validityTableForTransaction(const size_t& rows,
						  tx::transaction_id_t from_tid,
						  tx::transaction_id_t to_tid=VISIBLE);

}}


#endif
