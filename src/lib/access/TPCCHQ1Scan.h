// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_TPCCHQ1SCAN_H_
#define SRC_LIB_ACCESS_TPCCHQ1SCAN_H_

bool aaa_cmp(unsigned char *p, unsigned int i);

class TPCCHQ1Scan : public _PlanOperation {

 public:

  void executePlanOperation() {
    //6 is field
    auto table = input.getTable();

    pos_list_t *pos_list = new pos_list_t();
    pos_list->reserve(table->size());

    typedef unsigned char byte;

    std::vector<byte *> pointers;
    std::vector<size_t> widths;

    pointers.push_back((byte *) table->atSlice(6, 0));
    widths.push_back(table->getSliceWidth(6));

    size_t ts = table->size();
    for (size_t i  = 0; i < ts; ++i) {
      if (aaa_cmp(pointers[0], 1)) {
        pos_list->push_back(i);
      }

      *pointers[0] += widths[0];
    }
    AbstractTable::SharedTablePtr result =
        PointerCalculatorFactory::createPointerCalculator(input.getTable(), nullptr, pos_list);

    addResult(result);
  }

  static std::string name() {
    return "";
  }
  const std::string vname() {
    return "";
  }

};


#endif  // SRC_LIB_ACCESS_TPCCHQ1SCAN_H_
