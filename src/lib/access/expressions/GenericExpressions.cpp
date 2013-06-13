#include "GenericExpressions.h"
#include "helper/make_unique.h"
#include "helper/vector_helpers.h"

#include "access/expressions/ExpressionRegistration.h"

namespace hyrise { namespace access {


namespace {
auto _ = Expressions::add<Store_FLV_F1_EQ_INT_AND_F2_EQ_INT_AND_F3_EQ_INT>("hyrise::Store_FLV_F1_EQ_INT_AND_F2_EQ_INT_AND_F3_EQ_INT");
}


Store_FLV_F1_EQ_INT_AND_F2_EQ_INT_AND_F3_EQ_INT::Store_FLV_F1_EQ_INT_AND_F2_EQ_INT_AND_F3_EQ_INT() {

}

pos_list_t* Store_FLV_F1_EQ_INT_AND_F2_EQ_INT_AND_F3_EQ_INT::match(const size_t start, const size_t stop) {

	auto pl = new pos_list_t;
	size_t lower = 0;

	// Iterate over all parts
	for(size_t part =0, part_size = _vector_f1.size(); part < part_size; ++part) {
		
		// Define the boundaries for this part
		auto rows_in_part = _vector_f1[part]->size();

		// Get the ValueIDs for each attribute in this part
		auto vid_v1 = _store->getValueIdForValue<hyrise_int_t>(f1, v1, false, part).valueId;
		auto vid_v2 = _store->getValueIdForValue<hyrise_int_t>(f2, v2, false, part).valueId;
		auto vid_v3 = _store->getValueIdForValue<hyrise_int_t>(f3, v3, false, part).valueId;

		register auto of1 = _of_f1[part];
		register auto of2 = _of_f2[part];
		register auto of3 = _of_f2[part];

		auto ve1 = _vector_f1[part];
		auto ve2 = _vector_f2[part];
		auto ve3 = _vector_f3[part];

		if (stop < (rows_in_part + lower))
			rows_in_part = stop + lower;

		for(size_t x=0; x < rows_in_part ; ++x){
			// Emit lower + x
			// Check the condition
			if (vid_v1 == ve1->getRef(of1, x) &&
				vid_v2 == ve2->getRef(of2, x) &&
				vid_v3 == ve3->getRef(of3, x)) {
				pl->push_back(lower + x);
			}
		}

		lower = rows_in_part;
	}
	return pl;
}
	
void Store_FLV_F1_EQ_INT_AND_F2_EQ_INT_AND_F3_EQ_INT::walk(const std::vector<hyrise::storage::c_atable_ptr_t> &l) {
	_store = std::dynamic_pointer_cast<const Store>(l[0]);

	// Extract All the Attribute Vectors
	auto tmp = _store->getAttributeVectors(f1);
	_vector_f1 = collect(tmp, [](attr_vector_offset_t& v) { return std::dynamic_pointer_cast<VectorType>(v.attribute_vector); });
	_of_f1 = collect(tmp, [](attr_vector_offset_t& v) { return v.attribute_offset; });
	
	tmp = _store->getAttributeVectors(f2);
	_vector_f2 = collect(tmp, [](attr_vector_offset_t& v) { return std::dynamic_pointer_cast<VectorType>(v.attribute_vector); });
	_of_f2 = collect(tmp, [](attr_vector_offset_t& v) { return v.attribute_offset; });
	
	tmp = _store->getAttributeVectors(f3);
	_vector_f3 = collect(tmp, [](attr_vector_offset_t& v) { return std::dynamic_pointer_cast<VectorType>(v.attribute_vector); });
	_of_f3 = collect(tmp, [](attr_vector_offset_t& v) { return v.attribute_offset; });



}

std::unique_ptr<Store_FLV_F1_EQ_INT_AND_F2_EQ_INT_AND_F3_EQ_INT> Store_FLV_F1_EQ_INT_AND_F2_EQ_INT_AND_F3_EQ_INT::parse(const Json::Value& data) {
	auto res = make_unique<Store_FLV_F1_EQ_INT_AND_F2_EQ_INT_AND_F3_EQ_INT>();
	
	res->f1 = data["f1"].asUInt();
	res->f2 = data["f2"].asUInt();
	res->f3 = data["f3"].asUInt();

	res->v1 = data["v1"].asUInt();
	res->v2 = data["v2"].asUInt();
	res->v3 = data["v3"].asUInt();

	return res;
}


}}