// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_IO_TXCONTEXT_H_
#define SRC_LIB_IO_TXCONTEXT_H_

#include <helper/types.h>

namespace hyrise {namespace tx{


struct TXContext {

	using id_t = hyrise::tx::transaction_id_t;

	hyrise::tx::transaction_id_t tid = hyrise::tx::UNKNOWN;
	hyrise::tx::transaction_id_t lastCid = hyrise::tx::UNKNOWN;
	hyrise::tx::transaction_id_t cid = hyrise::tx::UNKNOWN;

	TXContext() {}

	TXContext(id_t _tid, id_t _lastCid, id_t _cid = hyrise::tx::UNKNOWN):
	tid(_tid), lastCid(_lastCid), cid(_cid){}

};


}}

#endif //SRC_LIB_IO_TXCONTEXT_H_