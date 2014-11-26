/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_HSQL_BISON_PARSER_H_INCLUDED
# define YY_HSQL_BISON_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef HSQL_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define HSQL_DEBUG 1
#  else
#   define HSQL_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define HSQL_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined HSQL_DEBUG */
#if HSQL_DEBUG
extern int hsql_debug;
#endif
/* "%code requires" blocks.  */
#line 52 "bison_parser.y" /* yacc.c:1909  */


#ifndef YYtypeDEF_YY_SCANNER_T
#define YYtypeDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

#define YYSTYPE HSQL_STYPE


#line 63 "bison_parser.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef HSQL_TOKENTYPE
# define HSQL_TOKENTYPE
  enum hsql_tokentype
  {
    SQL_IDENTIFIER = 258,
    SQL_STRING = 259,
    SQL_FLOAT = 260,
    SQL_INT = 261,
    SQL_NOTEQUALS = 262,
    SQL_LESSEQ = 263,
    SQL_GREATEREQ = 264,
    SQL_INTERSECT = 265,
    SQL_TEMPORARY = 266,
    SQL_DISTINCT = 267,
    SQL_RESTRICT = 268,
    SQL_TRUNCATE = 269,
    SQL_ANALYZE = 270,
    SQL_BETWEEN = 271,
    SQL_CASCADE = 272,
    SQL_COLUMNS = 273,
    SQL_CONTROL = 274,
    SQL_DEFAULT = 275,
    SQL_EXPLAIN = 276,
    SQL_HISTORY = 277,
    SQL_NATURAL = 278,
    SQL_PRIMARY = 279,
    SQL_SCHEMAS = 280,
    SQL_SPATIAL = 281,
    SQL_BEFORE = 282,
    SQL_COLUMN = 283,
    SQL_CREATE = 284,
    SQL_DELETE = 285,
    SQL_DIRECT = 286,
    SQL_ESCAPE = 287,
    SQL_EXCEPT = 288,
    SQL_EXISTS = 289,
    SQL_GLOBAL = 290,
    SQL_HAVING = 291,
    SQL_IMPORT = 292,
    SQL_INSERT = 293,
    SQL_ISNULL = 294,
    SQL_OFFSET = 295,
    SQL_RENAME = 296,
    SQL_SCHEMA = 297,
    SQL_SELECT = 298,
    SQL_SORTED = 299,
    SQL_TABLES = 300,
    SQL_UNIQUE = 301,
    SQL_UNLOAD = 302,
    SQL_UPDATE = 303,
    SQL_VALUES = 304,
    SQL_AFTER = 305,
    SQL_ALTER = 306,
    SQL_CROSS = 307,
    SQL_GROUP = 308,
    SQL_INDEX = 309,
    SQL_INNER = 310,
    SQL_LIMIT = 311,
    SQL_LOCAL = 312,
    SQL_MINUS = 313,
    SQL_ORDER = 314,
    SQL_OUTER = 315,
    SQL_RIGHT = 316,
    SQL_TABLE = 317,
    SQL_UNION = 318,
    SQL_USING = 319,
    SQL_WHERE = 320,
    SQL_CALL = 321,
    SQL_DESC = 322,
    SQL_DROP = 323,
    SQL_FILE = 324,
    SQL_FROM = 325,
    SQL_FULL = 326,
    SQL_HASH = 327,
    SQL_INTO = 328,
    SQL_JOIN = 329,
    SQL_LEFT = 330,
    SQL_LIKE = 331,
    SQL_LOAD = 332,
    SQL_NULL = 333,
    SQL_PLAN = 334,
    SQL_SHOW = 335,
    SQL_WITH = 336,
    SQL_ADD = 337,
    SQL_ALL = 338,
    SQL_AND = 339,
    SQL_ASC = 340,
    SQL_CSV = 341,
    SQL_FOR = 342,
    SQL_KEY = 343,
    SQL_NOT = 344,
    SQL_SET = 345,
    SQL_TBL = 346,
    SQL_TOP = 347,
    SQL_AS = 348,
    SQL_BY = 349,
    SQL_IF = 350,
    SQL_IN = 351,
    SQL_IS = 352,
    SQL_ON = 353,
    SQL_OR = 354,
    SQL_TO = 355,
    SQL_EQUALS = 356,
    SQL_LESS = 357,
    SQL_GREATER = 358,
    SQL_NOTNULL = 359,
    SQL_UMINUS = 360
  };
#endif

/* Value type.  */
#if ! defined HSQL_STYPE && ! defined HSQL_STYPE_IS_DECLARED
typedef union HSQL_STYPE HSQL_STYPE;
union HSQL_STYPE
{
#line 74 "bison_parser.y" /* yacc.c:1909  */

	double fval;
	int64_t ival;
	char* sval;
	uint uval;
	bool bval;

	hsql::Statement* statement;
	hsql::SelectStatement* select_stmt;
	hsql::ImportStatement* import_stmt;
	hsql::CreateStatement* create_stmt;
	hsql::InsertStatement* insert_stmt;

	hsql::TableRef* table;
	hsql::Expr* expr;
	hsql::OrderDescription* order;
	hsql::OrderType order_type;
	hsql::LimitDescription* limit;

	hsql::StatementList* stmt_list;
	hsql::List<char*>* slist;
	hsql::List<hsql::Expr*>* expr_list;
	hsql::List<hsql::TableRef*>* table_list;

#line 206 "bison_parser.h" /* yacc.c:1909  */
};
# define HSQL_STYPE_IS_TRIVIAL 1
# define HSQL_STYPE_IS_DECLARED 1
#endif



int hsql_parse (hsql::StatementList** result, yyscan_t scanner);

#endif /* !YY_HSQL_BISON_PARSER_H_INCLUDED  */
