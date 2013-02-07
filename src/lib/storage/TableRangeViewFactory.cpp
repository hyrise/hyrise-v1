// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * TableRangeViewFactory.cpp
 *
 *  Created on: Jan 14, 2013
 *      Author: jwust
 */

#include "TableRangeViewFactory.h"

std::shared_ptr<TableRangeView> TableRangeViewFactory::createView(hyrise::storage::atable_ptr_t _table, size_t start, size_t end) {
  return std::make_shared<TableRangeView>(_table, start, end);
}
