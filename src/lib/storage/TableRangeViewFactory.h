/*
 * TableRangeViewFactory.h
 *
 *  Created on: Jan 14, 2013
 *      Author: jwust
 */

#ifndef TABLERANGEVIEWFACTORY_H_
#define TABLERANGEVIEWFACTORY_H_

#include <memory>
#include "storage/AbstractTable.h"
#include "storage/TableRangeView.h"

#include "helper/types.h"

class TableRangeViewFactory {
public:
  TableRangeViewFactory(){};

  static std::shared_ptr<TableRangeView> createView(hyrise::storage::atable_ptr_t _table, size_t start, size_t end);
};

#endif /* TABLERANGEVIEWFACTORY_H_ */
