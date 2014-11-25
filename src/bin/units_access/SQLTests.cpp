// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/sql/SQLQueryParser.h"
#include "io/shortcuts.h"
#include "testing/test.h"


using namespace hsql;


namespace hyrise {
namespace access {
namespace sql {


#define EXPECT_NULL(pointer) EXPECT_TRUE(pointer == NULL);
#define EXPECT_NOTNULL(pointer) EXPECT_TRUE(pointer != NULL);

class SQLTests : public AccessTest {};

TEST_F(SQLTests, grammar_test) {
	std::vector<std::string> valid_queries;
	valid_queries.push_back("SELECT * FROM test;");
	valid_queries.push_back("SELECT name, address, age FROM customers WHERE age > 10 AND city = 'Berlin';");
	valid_queries.push_back("SELECT * FROM customers JOIN orders ON customers.id = orders.customer_id ORDER BY order_value;");


	for (std::string query : valid_queries) {
		StatementList* result = SQLParser::parseSQLString(query.c_str());
		EXPECT_TRUE(result->isValid);
		if (!result->isValid) fprintf(stderr, "Parsing failed: %s (%s)\n", query.c_str(), result->parser_msg);
		delete result;
	}



	std::vector<std::string> faulty_queries;
	faulty_queries.push_back("SELECT * FROM (SELECT * FROM test);"); // Missing alias for subquery

	for (std::string query : faulty_queries) {
		StatementList* result = SQLParser::parseSQLString(query.c_str());
		EXPECT_FALSE(result->isValid);
		if (result->isValid) fprintf(stderr, "Parsing shouldn't have succeeded: %s\n", query.c_str());
		delete result;
	}
}



TEST_F(SQLTests, select_parser_test) {
	std::string query = "SELECT customer_id, SUM(order_value) FROM customers JOIN orders ON customers.id = orders.customer_id GROUP BY customer_id ORDER BY SUM(order_value) DESC;";

	StatementList* list = SQLParser::parseSQLString(query.c_str());
	EXPECT_TRUE(list->isValid);
	if (!list->isValid) fprintf(stderr, "Parsing failed: %s (%s)\n", query.c_str(), list->parser_msg);

	
	EXPECT_EQ(list->size(), 1);
	EXPECT_EQ(list->at(0)->type, kStmtSelect);

	SelectStatement* stmt = (SelectStatement*) list->at(0);

	EXPECT_NOTNULL(stmt->select_list);
	EXPECT_NOTNULL(stmt->from_table);
	EXPECT_NOTNULL(stmt->group_by);
	EXPECT_NOTNULL(stmt->order);

	EXPECT_NULL(stmt->where_clause);
	EXPECT_NULL(stmt->union_select);
	EXPECT_NULL(stmt->limit);

	// Select List
	EXPECT_EQ(stmt->select_list->size(), 2);
	EXPECT_EQ(stmt->select_list->at(0)->type, kExprColumnRef);
	EXPECT_STREQ(stmt->select_list->at(0)->name, "customer_id");
	EXPECT_EQ(stmt->select_list->at(1)->type, kExprFunctionRef);
	EXPECT_STREQ(stmt->select_list->at(1)->name, "SUM");
	EXPECT_STREQ(stmt->select_list->at(1)->expr->name, "order_value");

	// Join Table
	JoinDefinition* join = stmt->from_table->join;
	EXPECT_EQ(stmt->from_table->type, kTableJoin);
	EXPECT_NOTNULL(join);
	EXPECT_STREQ(join->left->name, "customers");
	EXPECT_STREQ(join->right->name, "orders");
	EXPECT_EQ(join->condition->type, kExprOperator);
	EXPECT_STREQ(join->condition->expr->name, "id");
	EXPECT_STREQ(join->condition->expr->table, "customers");
	EXPECT_STREQ(join->condition->expr2->name, "customer_id");
	EXPECT_STREQ(join->condition->expr2->table, "orders");

	// Group By
	EXPECT_EQ(stmt->group_by->size(), 1);
	EXPECT_STREQ(stmt->group_by->at(0)->name, "customer_id");

	// Order By
	EXPECT_EQ(stmt->order->type, kOrderDesc);
	EXPECT_EQ(stmt->order->expr->type, kExprFunctionRef);
	EXPECT_STREQ(stmt->order->expr->name, "SUM");
	EXPECT_STREQ(stmt->order->expr->expr->name, "order_value");
}


} // namespace sql
} // namespace access
} // namespace hyrise
