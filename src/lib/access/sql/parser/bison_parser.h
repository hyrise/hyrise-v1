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
    SQL_DATABASE = 265,
    SQL_DISTINCT = 266,
    SQL_BETWEEN = 267,
    SQL_CONTROL = 268,
    SQL_NATURAL = 269,
    SQL_COLUMN = 270,
    SQL_CREATE = 271,
    SQL_DELETE = 272,
    SQL_EXISTS = 273,
    SQL_HAVING = 274,
    SQL_IMPORT = 275,
    SQL_INSERT = 276,
    SQL_ISNULL = 277,
    SQL_OFFSET = 278,
    SQL_RENAME = 279,
    SQL_SELECT = 280,
    SQL_UNLOAD = 281,
    SQL_UPDATE = 282,
    SQL_ALTER = 283,
    SQL_CROSS = 284,
    SQL_GROUP = 285,
    SQL_INDEX = 286,
    SQL_INNER = 287,
    SQL_LIMIT = 288,
    SQL_ORDER = 289,
    SQL_OUTER = 290,
    SQL_RIGHT = 291,
    SQL_TABLE = 292,
    SQL_UNION = 293,
    SQL_USING = 294,
    SQL_WHERE = 295,
    SQL_DESC = 296,
    SQL_DROP = 297,
    SQL_FILE = 298,
    SQL_FROM = 299,
    SQL_INTO = 300,
    SQL_JOIN = 301,
    SQL_LEFT = 302,
    SQL_LIKE = 303,
    SQL_LOAD = 304,
    SQL_NULL = 305,
    SQL_ALL = 306,
    SQL_AND = 307,
    SQL_ASC = 308,
    SQL_CSV = 309,
    SQL_NOT = 310,
    SQL_TBL = 311,
    SQL_TOP = 312,
    SQL_AS = 313,
    SQL_BY = 314,
    SQL_IF = 315,
    SQL_IN = 316,
    SQL_IS = 317,
    SQL_ON = 318,
    SQL_OR = 319,
    SQL_EQUALS = 320,
    SQL_LESS = 321,
    SQL_GREATER = 322,
    SQL_NOTNULL = 323,
    SQL_UMINUS = 324
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

	hsql::TableRef* table;
	hsql::Expr* expr;
	hsql::OrderDescription* order;
	hsql::OrderType order_type;
	hsql::LimitDescription* limit;

	hsql::StatementList* stmt_list;
	hsql::List<char*>* slist;
	hsql::List<hsql::Expr*>* expr_list;
	hsql::List<hsql::TableRef*>* table_list;

#line 169 "bison_parser.h" /* yacc.c:1909  */
};
# define HSQL_STYPE_IS_TRIVIAL 1
# define HSQL_STYPE_IS_DECLARED 1
#endif



int hsql_parse (hsql::StatementList** result, yyscan_t scanner);

#endif /* !YY_HSQL_BISON_PARSER_H_INCLUDED  */
