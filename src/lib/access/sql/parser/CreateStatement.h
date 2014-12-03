#ifndef __CREATE_STATEMENT_H__
#define __CREATE_STATEMENT_H__

#include "SQLStatement.h"

namespace hsql {


/**
 * @struct ColumnDefinition
 */
struct ColumnDefinition {
	enum DataType {
		TEXT,
		INT,
		DOUBLE
	};

	ColumnDefinition(char* name, DataType type) :
		name(name),
		type(type) {}

	virtual ~ColumnDefinition(); // defined in destructors.cpp

	char* name;
	DataType type;
};

/**
 * @struct CreateStatement
 * CREATE TABLE students (name TEXT, student_number INTEGER, city TEXT, grade DOUBLE)
 * CREATE TABLE students FROM TBL FILE 'test/students.tbl'
 */
struct CreateStatement : SQLStatement {
	enum CreateType {
		kTable,
		kTableFromTbl, // Hyrise file format
	};

	CreateStatement(CreateType type) :
		SQLStatement(kStmtCreate),
		type(type),
		if_not_exists(false),
		columns(NULL),
		file_path(NULL),
		table_name(NULL) {};

	virtual ~CreateStatement(); // defined in destructors.cpp

	CreateType type;
	bool if_not_exists;

	List<ColumnDefinition*>* columns;

	const char* file_path;
	const char* table_name;
};



} // namespace hsql
#endif