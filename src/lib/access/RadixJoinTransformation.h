// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCES_RADIXJOINTRANSFORMATION_H
#define SRC_LIB_ACCES_RADIXJOINTRANSFORMATION_H

#include "access/RadixJoin.h"
#include <json.h>
#include "access/AbstractPlanOpTransformation.h"

namespace hyrise {
namespace access {

 /*
  * This class transforms a virtual operator RadixJoin into a set of operators that perform the radix join.
  * The transformation is based on a Json query that is rewritten; the actual operators are instatiated at a later stage.
  * Currently only sequential plans are supported!!!
  */
class RadixJoinTransformation: public AbstractPlanOpTransformation {
  static bool is_registered;
  
  void appendEdge(const std::string &srcId,const std::string &dstId,Json::Value &query) const;
  void removeOperator(Json::Value &query,const Json::Value &operatorId) const;
  std::vector<Json::Value> createFirstPass(const Json::Value &radix);
  std::vector<Json::Value> createSecondPass(const Json::Value &radix);
  Json::Value createJoin(const Json::Value &radix);
  void connectFirstPassWithInput(std::vector<std::string> &pass, std::string input, Json::Value &query);
  void connectFirstPass(std::vector<std::string> &pass, Json::Value &query);
  void connectSecondPass(std::vector<std::string> &pass, Json::Value &query);
  void connectFirstAndSecondPass(std::vector<std::string> &firstpass, std::vector<std::string> &secondpass, Json::Value &query);
  void connectFirstPassWithJoin(std::vector<std::string> &pass, std::string join, Json::Value &query);
  void connectSecondPassWithJoin(std::vector<std::string> &pass, std::string join, Json::Value &query);
  void connectJoinWithInput(std::string input, std::string join, Json::Value &query);

public:
  RadixJoinTransformation(){};
  virtual ~RadixJoinTransformation(){};

  void transform(Json::Value &op, const std::string &operatorId, Json::Value &query);

  static const std::string name() {
    return "RadixJoin";
  }

};

}
}

#endif // SRC_LIB_ACCES_RADIXJOINTRANSFORMATION_H