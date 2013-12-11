#include "boost/lexical_cast.hpp"

#include "JsonTable.h"


#include <helper/types.h>
#include <helper/vector_helpers.h>
#include <helper/stringhelpers.h>

#include <io/TransactionManager.h>

#include <storage/storage_types.h>
#include <storage/meta_storage.h>
#include <storage/Store.h>
#include <storage/Serial.h>
#include <storage/TableBuilder.h>

#include <io/ResourceManager.h>

#include <algorithm>
#include <iterator>
#include <functional>

namespace hyrise { namespace access	{

namespace {
  auto _ = QueryParser::registerPlanOperation<JsonTable>("JsonTable");
}

JsonTable::JsonTable() : _useStoreFlag(false) {

}

struct set_string_value_functor {

	typedef void value_type;

	storage::atable_ptr_t tab;
	size_t col;
	size_t row;
	std::string val;

	set_string_value_functor(storage::atable_ptr_t t): tab(t), col(0), row(0) {
	}

	inline void set(size_t c, size_t r, std::string& v) {
		row =r; col = c; val = v;
	}

	template<typename T>
	inline value_type operator()() {
		tab->setValue(col, row, boost::lexical_cast<T>(val));
	}

};

void JsonTable::executePlanOperation() {

  storage::TableBuilder::param_list list;

	for(size_t i=0, attr_size = _names.size(); i < attr_size; ++i)
		list.append().set_name(_names[i]).set_type(_types[i]);

	for(size_t i=0, groups_size = _groups.size(); i < groups_size; ++i)
		list.appendGroup(_groups[i]);


	storage::atable_ptr_t result = storage::TableBuilder::build(list);
	if (_useStoreFlag) {
		result = std::make_shared<storage::Store>(result);
	}

	// Attach all Serial Fields
	// Get resource manager
	auto& res_man = io::ResourceManager::getInstance();
	for(const auto& f : _serialFields) {
		auto serial_name = std::to_string(result->getUuid()) + "_" + f;
		res_man.add(serial_name, std::make_shared<storage::Serial>());
	}

	// Add the rows if any
	size_t rows = _data.size();
	if (rows > 0 ) {

		if (_useStoreFlag)
			std::dynamic_pointer_cast<storage::Store>(result)->appendToDelta(rows);
		else
			result->resize(rows);


		set_string_value_functor fun(_useStoreFlag ? std::dynamic_pointer_cast<storage::Store>(result)->getDeltaTable() : result);
		storage::type_switch<hyrise_basic_types> ts;


		for(size_t i=0; i < rows; ++i) {
			auto row = _data[i];

			if ((row.size() + _serialFields.size()) != _names.size())
				throw std::runtime_error("Mismatch in provided data and number of columns in table!");

			// Handle the insertion of all rows and manage the offsets given by the serial fields
			size_t offset = 0;
			for(size_t j=0, rs = _names.size(); j < rs; ++j) {
				if (std::find(_serialFields.begin(), _serialFields.end(), _names[j]) != _serialFields.end()) {
					auto serial_name = std::to_string(result->getUuid()) + "_" + _names[j];
					auto ser = res_man.get<storage::Serial>(serial_name);
					result->setValue<hyrise_int_t>(j, i, ser->next());
					offset++;
				} else {
					fun.set(j, i, row[j-offset]);
					ts(result->typeOfColumn(j), fun);
				}
			}
		}

		if (_useStoreFlag) {
			// Hijacking transactions
			pos_list_t pl(rows);
			size_t counter = 0;
			std::generate(pl.begin(), pl.end(), [&counter](){
				return counter++;
			});

			// This can be dangerous..., but as JSONTable with data will be probably
			// only used for tests, this might be OK.
			std::dynamic_pointer_cast<storage::Store>(result)->commitPositions(pl,tx::UNKNOWN_CID, true);
			
			if (_mergeFlag)
				std::dynamic_pointer_cast<storage::Store>(result)->merge();
		}
	}

	addResult(result);
}

std::shared_ptr<PlanOperation> JsonTable::parse(const Json::Value &data) {
	auto result = std::make_shared<JsonTable>();

	result->_names = functional::collect(data["names"], [](const Json::Value& v) { return v.asString();});
	result->_types = functional::collect(data["types"], [](const Json::Value& v) { return v.asString();});
	result->_groups = functional::collect(data["groups"], [](const Json::Value& v) { return v.asUInt();});

	if (data.isMember("data")) {
		result->_data = functional::collect(data["data"], [](const Json::Value& v){
			return functional::collect(v, [](const Json::Value& c){ return c.asString(); });
		});
	}

	if (data.isMember("useStore")) {
		result->setUseStore(data["useStore"].asBool());
		result->setMergeStore(data["mergeStore"].asBool());
	}

	// Check if we have serial field definitions
	if (data.isMember("serials")) {
		result->_serialFields = functional::collect(data["serials"], [](const Json::Value& v) { return v.asString();});
	}

	return result;
}

void JsonTable::setData(const std::vector<std::vector<std::string>> d) {
	_data = d;
}

void JsonTable::setNames(const std::vector<std::string> data) {
	_names = data;
}

void JsonTable::setTypes(const std::vector<std::string> data) {
	_types = data;
}

void JsonTable::setGroups(const std::vector<unsigned> data) {
	_groups = data;
}

void JsonTable::setUseStore(const bool v) {
	_useStoreFlag = v;
}

void JsonTable::setMergeStore(const bool v) {
	_mergeFlag = v;
}
}}
