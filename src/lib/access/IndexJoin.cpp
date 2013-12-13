#include "IndexJoin.h"

#include <set>
#include <functional>

#include "io/StorageManager.h"

#include "storage/PointerCalculator.h"
#include "storage/storage_types.h"
#include "storage/InvertedIndex.h"
#include "storage/meta_storage.h"
#include "storage/PointerCalculator.h"


#include "access/system/QueryParser.h"
#include "access/expressions/ExpressionRegistration.h"

namespace hyrise { namespace access {
namespace { 
auto _ = QueryParser::registerPlanOperation<IndexJoin>("IndexJoin");
}


struct IndexJoinFunctor {
  typedef storage::c_atable_ptr_t value_type;

  std::shared_ptr<storage::AbstractIndex> _index;
  unsigned _field;
  const storage::c_atable_ptr_t& _left;
  const storage::c_atable_ptr_t& _right;
  const storage::pos_list_t *_input;

  IndexJoinFunctor(std::string idxName, unsigned f, const storage::c_atable_ptr_t& t, const storage::pos_list_t *i, const storage::c_atable_ptr_t &r): 
    _index(io::StorageManager::getInstance()->getInvertedIndex(idxName)),
     _field(f), _left(t), _right(r), _input(i) {
  
  }

  template<typename R>
  value_type operator()() {
    auto idx = std::dynamic_pointer_cast<storage::InvertedIndex<R>>(_index);

    auto lpos_list = new pos_list_t;
    auto rpos_list = new pos_list_t;

    auto size = _input->size();
    for(size_t r=0; r < size; ++r) {
      const auto& tmp = idx->getPositionsForKey(_left->getValue<R>(_field, (*_input)[r]));
      auto tmp_size = tmp.size();
      for(size_t i=0; i < tmp_size; ++i) {
        lpos_list->push_back((*_input)[r]);
        rpos_list->push_back(tmp[i]);
      }
    }

    auto l = storage::PointerCalculator::create(_left, lpos_list);
    auto r = storage::PointerCalculator::create(_right, rpos_list);

    // We need to check if there are duplicate fields
    std::set<std::string> checkFields;
    bool updateFields = false;
    for(size_t i=0; i < l->columnCount(); ++i){
      if (checkFields.count(l->nameOfColumn(i)) == 0)
        checkFields.insert(l->nameOfColumn(i));
      else {
        updateFields = true;
        break;
      }
    }

    for(size_t i=0; !updateFields && i < r->columnCount(); ++i){
      if (checkFields.count(r->nameOfColumn(i)) == 0)
        checkFields.insert(r->nameOfColumn(i));
      else {
        updateFields = true;
        break;
      }
    }

    for(size_t i=0; updateFields && i < l->columnCount(); ++i)
      l->rename(i, l->nameOfColumn(i) + "_1");
    for(size_t i=0; updateFields && i < r->columnCount(); ++i)
      r->rename(i, r->nameOfColumn(i) + "_2");
    
    return std::make_shared<storage::MutableVerticalTable>(
        std::vector<storage::atable_ptr_t>({l,r}));
  }

};

std::shared_ptr<PlanOperation> IndexJoin::parse(const Json::Value &data) {
auto res = BasicParser<IndexJoin>::parse(data);
  res->_indexName = data["index"].asString();
  return res;
}



/**
 * The Algorithm for the join is as follows: First rextract the two
 * input tables and make sure they are of the required type. Then, for
 * each of the left rows perform the lookup of the values on the right
 * side.
 */
void IndexJoin::executePlanOperation() {
  auto tmp = std::dynamic_pointer_cast<const storage::PointerCalculator>(getInputTable(0));
  _left = tmp->getActualTable();
  _right = getInputTable(1);
  _tab_pos_list = tmp->getPositions();

  storage::type_switch<hyrise_basic_types> ts;
  IndexJoinFunctor fun(_indexName, _field_definition[0], _left, _tab_pos_list, _right);
  addResult(ts(_left->typeOfColumn(_field_definition[0]), fun));
}



}}
