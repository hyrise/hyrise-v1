#ifndef SRC_LIB_ACCESS_EXPRESSIONS_GENERICEXPRESSIONS_INC_
#define SRC_LIB_ACCESS_EXPRESSIONS_GENERICEXPRESSIONS_INC_

#include <boost/preprocessor.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>



#define EXPRESSION_ADD_REFERENCE_FIELD(r, data, seq_field) \
	std::vector<SharedVectorType> BOOST_PP_CAT(_vector_, BOOST_PP_SEQ_ELEM(0, seq_field));	\
	std::vector<size_t> BOOST_PP_CAT(_of_, BOOST_PP_SEQ_ELEM(0, seq_field));\
	field_t BOOST_PP_SEQ_ELEM(0, seq_field);\
	BOOST_PP_SEQ_ELEM(1, seq_field) BOOST_PP_CAT(v_, BOOST_PP_SEQ_ELEM(0, seq_field));



#define EXPRESSION_EXTRACT_AV_AND_OFFSET(r, data, name) \
	const	auto& BOOST_PP_CAT(tmp, name) = _store->getAttributeVectors(name); \
	BOOST_PP_CAT(_vector_,name) = hyrise::functional::collect(BOOST_PP_CAT(tmp, name), [](const attr_vector_offset_t& v) { return std::dynamic_pointer_cast<VectorType>(v.attribute_vector); }); \
	BOOST_PP_CAT(_of_, name) = hyrise::functional::collect(BOOST_PP_CAT(tmp, name), [](const attr_vector_offset_t& v) { return v.attribute_offset; });


#define EXPR_DATA_VECTOR_NAME(name) BOOST_PP_CAT(_vector_, name)

#define EXPR_VALUE_VAR_NAME(name) BOOST_PP_CAT(v_,name)

#define EXPR_EXTRACT_VID_AND_MAP(r, data, seq_field) \
	auto BOOST_PP_CAT(vid_v_, BOOST_PP_SEQ_ELEM(0,seq_field)) = _store->getValueIdForValue< BOOST_PP_SEQ_ELEM(1,seq_field) >( BOOST_PP_SEQ_ELEM(0, seq_field), BOOST_PP_CAT(v_, BOOST_PP_SEQ_ELEM(0, seq_field)), false, part).valueId; \
	auto BOOST_PP_CAT(o, BOOST_PP_SEQ_ELEM(0,seq_field)) = BOOST_PP_CAT(_of_, BOOST_PP_SEQ_ELEM(0,seq_field))[part]; \
	auto BOOST_PP_CAT(ve_, BOOST_PP_SEQ_ELEM(0,seq_field)) = EXPR_DATA_VECTOR_NAME(BOOST_PP_SEQ_ELEM(0,seq_field))[part];

#define EXPR_COMPARE_FIELD(name, cmp, row) BOOST_PP_CAT(vid_v_,name) cmp BOOST_PP_CAT(ve_,name)->getRef(BOOST_PP_CAT(o,name), row)

#define EXPR_LOGICAL_TWO(op, left, right) (left op right)

#define EXPR_FIELD_VAL(seq_of_fields, i) BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(i,seq_of_fields))

#define EXPR_GET_EXPRESSION(r, data, i , seq_field) BOOST_PP_SEQ_ELEM(i, data) EXPR_COMPARE_FIELD( BOOST_PP_SEQ_ELEM(0, seq_field), BOOST_PP_SEQ_ELEM(2, seq_field) , row)

#define EXPR_BUILD_LOOP(pl, seq_of_fields, seq_logic) \
	auto pl = new pos_list_t; \
	size_t lower = 0; \
	pl->clear(); \
	const auto& first_vector = EXPR_DATA_VECTOR_NAME(EXPR_FIELD_VAL(seq_of_fields, 0)); \
	size_t part_size = first_vector.size();\
	part_size -= _with_delta ? 0 : 1;\
	for(size_t part =0; part < part_size; ++part) {\
		auto rows_in_part = first_vector[part]->size(); \
		BOOST_PP_SEQ_FOR_EACH(EXPR_EXTRACT_VID_AND_MAP,, seq_of_fields);\
		if (stop < (rows_in_part + lower)) \
			rows_in_part = stop + lower; \
		for(size_t row=0; row < rows_in_part ; ++row){\
			if ( BOOST_PP_SEQ_FOR_EACH_I(EXPR_GET_EXPRESSION, seq_logic, seq_of_fields) ) { \
	 			pl->push_back(lower + row);\
			}\
 		}\
	}

#define EXPR_EXTRACT_FFROM_JSON(r, data, seq_field) \
	data->BOOST_PP_SEQ_ELEM(0, seq_field) = val[BOOST_PP_STRINGIZE(BOOST_PP_SEQ_ELEM(0, seq_field))].asUInt();\
	data->BOOST_PP_CAT(v_, BOOST_PP_SEQ_ELEM(0, seq_field)) = val["v_" BOOST_PP_STRINGIZE(BOOST_PP_SEQ_ELEM(0, seq_field)) ].BOOST_PP_SEQ_ELEM(3, seq_field)();\

#define EXPR_TRANSFORM_FIELD_NAMES(s, data, elem) BOOST_PP_SEQ_ELEM(0, elem)


#define REGISTER_EXPRESSION_CLASS(name)\
	namespace { auto BOOST_PP_CAT(reg_, name) = Expressions::add<name>("hyrise::" BOOST_PP_STRINGIZE(name) ); } \


#define IMPLEMENT_EXPRESSION_CLASS(name, seq_of_fields, seq_logic) \
	name::name(){}\
	pos_list_t* name::match(const size_t start, const size_t stop) {\
		EXPR_BUILD_LOOP(the_result, seq_of_fields, seq_logic );\
		return the_result;\
	}\
	void name::walk(const std::vector<hyrise::storage::c_atable_ptr_t> &l) {\
		_store = std::dynamic_pointer_cast<const storage::Store>(l[0]);\
		BOOST_PP_SEQ_FOR_EACH(EXPRESSION_EXTRACT_AV_AND_OFFSET,,BOOST_PP_SEQ_TRANSFORM(EXPR_TRANSFORM_FIELD_NAMES,,seq_of_fields));\
	}\
	std::unique_ptr<name> name::parse(const Json::Value& val) {\
		auto res = make_unique<name>();\
		if (val.isMember("withDelta")) res->_with_delta = val["withDelta"].asBool();\
		BOOST_PP_SEQ_FOR_EACH(EXPR_EXTRACT_FFROM_JSON, res, seq_of_fields);\
		return res;\
	}\

#define DEFINE_EXPRESSION_CLASS(name, seq_of_fields,seq_logic) \
	class name : public AbstractExpression { \
		using VectorType = FixedLengthVector<value_id_t>;\
		using SharedVectorType = std::shared_ptr<VectorType>;\
		hyrise::storage::c_store_ptr_t _store;\
		bool _with_delta=true;\
		BOOST_PP_SEQ_FOR_EACH(EXPRESSION_ADD_REFERENCE_FIELD,,seq_of_fields);\
	public:\
		name(); \
		virtual pos_list_t* match(const size_t start, const size_t stop);\
		virtual void walk(const std::vector<hyrise::storage::c_atable_ptr_t> &l);		\
		static std::unique_ptr<name> parse(const Json::Value& data);\
	};\
	IMPLEMENT_EXPRESSION_CLASS(name, seq_of_fields, seq_logic);\





#endif // SRC_LIB_ACCESS_EXPRESSIONS_GENERICEXPRESSIONS_INC_