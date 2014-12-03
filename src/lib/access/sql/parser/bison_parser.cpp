/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Substitute the type names.  */
#define YYSTYPE         HSQL_STYPE
/* Substitute the variable and function names.  */
#define yyparse         hsql_parse
#define yylex           hsql_lex
#define yyerror         hsql_error
#define yydebug         hsql_debug
#define yynerrs         hsql_nerrs


/* Copy the first part of user declarations.  */
#line 1 "bison_parser.y" /* yacc.c:339  */

/**
 * bison_parser.y
 * defines bison_parser.h
 * outputs bison_parser.c
 * 
 * Grammar File Spec: http://dinosaur.compilertools.net/bison/bison_6.html
 *
 */
/*********************************
 ** Section 1: C Declarations
 *********************************/

#include "sqllib.h"
#include "bison_parser.h"
#include "flex_lexer.h"

#include <stdio.h>

using namespace hsql;

int yyerror(SQLStatementList** result, yyscan_t scanner, const char *msg) {

	SQLStatementList* list = new SQLStatementList();
	list->isValid = false;
	list->parser_msg = strdup(msg);
	*result = list;

	return 0;
}


#line 106 "bison_parser.cpp" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "bison_parser.h".  */
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
#line 52 "bison_parser.y" /* yacc.c:355  */


#ifndef YYtypeDEF_YY_SCANNER_T
#define YYtypeDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

#define YYSTYPE HSQL_STYPE


#line 155 "bison_parser.cpp" /* yacc.c:355  */

/* Token type.  */
#ifndef HSQL_TOKENTYPE
# define HSQL_TOKENTYPE
  enum hsql_tokentype
  {
    SQL_IDENTIFIER = 258,
    SQL_STRING = 259,
    SQL_FLOATVAL = 260,
    SQL_INTVAL = 261,
    SQL_NOTEQUALS = 262,
    SQL_LESSEQ = 263,
    SQL_GREATEREQ = 264,
    SQL_PARAMETERS = 265,
    SQL_INTERSECT = 266,
    SQL_TEMPORARY = 267,
    SQL_TIMESTAMP = 268,
    SQL_DISTINCT = 269,
    SQL_NVARCHAR = 270,
    SQL_RESTRICT = 271,
    SQL_TRUNCATE = 272,
    SQL_ANALYZE = 273,
    SQL_BETWEEN = 274,
    SQL_CASCADE = 275,
    SQL_COLUMNS = 276,
    SQL_CONTROL = 277,
    SQL_DEFAULT = 278,
    SQL_EXPLAIN = 279,
    SQL_HISTORY = 280,
    SQL_INTEGER = 281,
    SQL_NATURAL = 282,
    SQL_PRIMARY = 283,
    SQL_SCHEMAS = 284,
    SQL_SPATIAL = 285,
    SQL_VIRTUAL = 286,
    SQL_BEFORE = 287,
    SQL_COLUMN = 288,
    SQL_CREATE = 289,
    SQL_DELETE = 290,
    SQL_DIRECT = 291,
    SQL_DOUBLE = 292,
    SQL_ESCAPE = 293,
    SQL_EXCEPT = 294,
    SQL_EXISTS = 295,
    SQL_GLOBAL = 296,
    SQL_HAVING = 297,
    SQL_IMPORT = 298,
    SQL_INSERT = 299,
    SQL_ISNULL = 300,
    SQL_OFFSET = 301,
    SQL_RENAME = 302,
    SQL_SCHEMA = 303,
    SQL_SELECT = 304,
    SQL_SORTED = 305,
    SQL_TABLES = 306,
    SQL_UNIQUE = 307,
    SQL_UNLOAD = 308,
    SQL_UPDATE = 309,
    SQL_VALUES = 310,
    SQL_AFTER = 311,
    SQL_ALTER = 312,
    SQL_CROSS = 313,
    SQL_DELTA = 314,
    SQL_GROUP = 315,
    SQL_INDEX = 316,
    SQL_INNER = 317,
    SQL_LIMIT = 318,
    SQL_LOCAL = 319,
    SQL_MERGE = 320,
    SQL_MINUS = 321,
    SQL_ORDER = 322,
    SQL_OUTER = 323,
    SQL_RIGHT = 324,
    SQL_TABLE = 325,
    SQL_UNION = 326,
    SQL_USING = 327,
    SQL_WHERE = 328,
    SQL_CALL = 329,
    SQL_DATE = 330,
    SQL_DESC = 331,
    SQL_DROP = 332,
    SQL_FILE = 333,
    SQL_FROM = 334,
    SQL_FULL = 335,
    SQL_HASH = 336,
    SQL_INTO = 337,
    SQL_JOIN = 338,
    SQL_LEFT = 339,
    SQL_LIKE = 340,
    SQL_LOAD = 341,
    SQL_NULL = 342,
    SQL_PART = 343,
    SQL_PLAN = 344,
    SQL_SHOW = 345,
    SQL_TEXT = 346,
    SQL_TIME = 347,
    SQL_VIEW = 348,
    SQL_WITH = 349,
    SQL_ADD = 350,
    SQL_ALL = 351,
    SQL_AND = 352,
    SQL_ASC = 353,
    SQL_CSV = 354,
    SQL_FOR = 355,
    SQL_INT = 356,
    SQL_KEY = 357,
    SQL_NOT = 358,
    SQL_OFF = 359,
    SQL_SET = 360,
    SQL_TBL = 361,
    SQL_TOP = 362,
    SQL_AS = 363,
    SQL_BY = 364,
    SQL_IF = 365,
    SQL_IN = 366,
    SQL_IS = 367,
    SQL_OF = 368,
    SQL_ON = 369,
    SQL_OR = 370,
    SQL_TO = 371,
    SQL_EQUALS = 372,
    SQL_LESS = 373,
    SQL_GREATER = 374,
    SQL_NOTNULL = 375,
    SQL_UMINUS = 376
  };
#endif

/* Value type.  */
#if ! defined HSQL_STYPE && ! defined HSQL_STYPE_IS_DECLARED
typedef union HSQL_STYPE HSQL_STYPE;
union HSQL_STYPE
{
#line 74 "bison_parser.y" /* yacc.c:355  */

	double fval;
	int64_t ival;
	char* sval;
	uint uval;
	bool bval;

	hsql::SQLStatement* statement;
	hsql::SelectStatement* select_stmt;
	hsql::ImportStatement* import_stmt;
	hsql::CreateStatement* create_stmt;
	hsql::InsertStatement* insert_stmt;
	hsql::DeleteStatement* delete_stmt;
	hsql::UpdateStatement* update_stmt;
	hsql::DropStatement*   drop_stmt;

	hsql::TableRef* table;
	hsql::Expr* expr;
	hsql::OrderDescription* order;
	hsql::OrderType order_type;
	hsql::LimitDescription* limit;
	hsql::ColumnDefinition* column_t;
	hsql::UpdateClause* update_t;

	hsql::SQLStatementList* stmt_list;
	hsql::List<char*>* slist;
	hsql::List<hsql::Expr*>* expr_list;
	hsql::List<hsql::TableRef*>* table_list;
	hsql::List<hsql::ColumnDefinition*>* column_list_t;
	hsql::List<hsql::UpdateClause*>* update_list_t;

#line 321 "bison_parser.cpp" /* yacc.c:355  */
};
# define HSQL_STYPE_IS_TRIVIAL 1
# define HSQL_STYPE_IS_DECLARED 1
#endif



int hsql_parse (hsql::SQLStatementList** result, yyscan_t scanner);

#endif /* !YY_HSQL_BISON_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 335 "bison_parser.cpp" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined HSQL_STYPE_IS_TRIVIAL && HSQL_STYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  58
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   356

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  138
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  61
/* YYNRULES -- Number of rules.  */
#define YYNRULES  131
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  230

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   376

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   128,     2,     2,
     133,   134,   126,   124,   137,   125,   135,   127,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   136,
     119,   117,   120,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   131,     2,   132,   129,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   118,   121,   122,   123,   130
};

#if HSQL_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   193,   193,   198,   199,   203,   204,   205,   206,   207,
     208,   209,   210,   219,   227,   231,   241,   247,   256,   257,
     261,   262,   266,   273,   274,   275,   276,   285,   296,   304,
     316,   322,   332,   333,   343,   352,   353,   357,   369,   370,
     374,   375,   379,   384,   396,   397,   398,   402,   413,   418,
     423,   424,   429,   430,   435,   436,   440,   441,   442,   447,
     448,   449,   456,   457,   461,   462,   466,   473,   474,   475,
     476,   477,   481,   482,   483,   487,   488,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,   502,   507,   508,
     509,   510,   511,   512,   516,   520,   521,   525,   526,   530,
     535,   536,   540,   544,   552,   553,   563,   564,   570,   575,
     576,   581,   591,   599,   600,   605,   606,   610,   611,   619,
     631,   632,   633,   634,   635,   641,   647,   651,   660,   661,
     666,   667
};
#endif

#if HSQL_DEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENTIFIER", "STRING", "FLOATVAL",
  "INTVAL", "NOTEQUALS", "LESSEQ", "GREATEREQ", "PARAMETERS", "INTERSECT",
  "TEMPORARY", "TIMESTAMP", "DISTINCT", "NVARCHAR", "RESTRICT", "TRUNCATE",
  "ANALYZE", "BETWEEN", "CASCADE", "COLUMNS", "CONTROL", "DEFAULT",
  "EXPLAIN", "HISTORY", "INTEGER", "NATURAL", "PRIMARY", "SCHEMAS",
  "SPATIAL", "VIRTUAL", "BEFORE", "COLUMN", "CREATE", "DELETE", "DIRECT",
  "DOUBLE", "ESCAPE", "EXCEPT", "EXISTS", "GLOBAL", "HAVING", "IMPORT",
  "INSERT", "ISNULL", "OFFSET", "RENAME", "SCHEMA", "SELECT", "SORTED",
  "TABLES", "UNIQUE", "UNLOAD", "UPDATE", "VALUES", "AFTER", "ALTER",
  "CROSS", "DELTA", "GROUP", "INDEX", "INNER", "LIMIT", "LOCAL", "MERGE",
  "MINUS", "ORDER", "OUTER", "RIGHT", "TABLE", "UNION", "USING", "WHERE",
  "CALL", "DATE", "DESC", "DROP", "FILE", "FROM", "FULL", "HASH", "INTO",
  "JOIN", "LEFT", "LIKE", "LOAD", "NULL", "PART", "PLAN", "SHOW", "TEXT",
  "TIME", "VIEW", "WITH", "ADD", "ALL", "AND", "ASC", "CSV", "FOR", "INT",
  "KEY", "NOT", "OFF", "SET", "TBL", "TOP", "AS", "BY", "IF", "IN", "IS",
  "OF", "ON", "OR", "TO", "'='", "EQUALS", "'<'", "'>'", "LESS", "GREATER",
  "NOTNULL", "'+'", "'-'", "'*'", "'/'", "'%'", "'^'", "UMINUS", "'['",
  "']'", "'('", "')'", "'.'", "';'", "','", "$accept", "input",
  "statement_list", "statement", "import_statement", "import_file_type",
  "file_path", "create_statement", "opt_not_exists",
  "column_def_commalist", "column_def", "column_type", "drop_statement",
  "delete_statement", "truncate_statement", "insert_statement",
  "opt_column_list", "update_statement", "update_clause_commalist",
  "update_clause", "select_statement", "select_with_paren",
  "select_no_paren", "set_operator", "select_clause", "select_list",
  "from_clause", "opt_where", "opt_group", "opt_order", "opt_order_type",
  "opt_limit", "expr_list", "literal_list", "expr_alias", "expr",
  "scalar_expr", "unary_expr", "binary_expr", "comp_expr", "function_expr",
  "column_name", "literal", "string_literal", "num_literal", "int_literal",
  "star_expr", "table_ref", "table_ref_atomic", "table_ref_commalist",
  "table_ref_name", "table_ref_name_no_alias", "table_name", "alias",
  "opt_alias", "join_clause", "opt_join_type", "join_table",
  "join_condition", "opt_semicolon", "ident_commalist", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,    61,   372,    60,
      62,   373,   374,   375,    43,    45,    42,    47,    37,    94,
     376,    91,    93,    40,    41,    46,    59,    44
};
# endif

#define YYPACT_NINF -158

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-158)))

#define YYTABLE_NINF -127

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-127)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     122,    19,   -44,   -50,   -49,   -46,     1,    19,   -33,   -38,
      46,   -65,  -158,  -158,  -158,  -158,  -158,  -158,  -158,  -158,
    -158,  -158,  -158,    27,   -57,  -158,   -31,    19,   -11,    19,
     -42,  -158,  -158,  -158,     1,     1,  -158,     1,    18,   -37,
    -158,   100,  -158,  -158,  -158,  -158,  -158,  -158,  -158,  -158,
    -158,  -158,  -158,    -6,  -158,    19,   -29,   -20,  -158,   122,
    -158,  -158,  -158,     8,  -158,    74,    53,   121,    22,    19,
      55,  -158,    51,    -3,     1,   134,   198,   -84,    16,    -1,
      55,     1,  -158,     1,     1,     1,     1,     1,    61,   144,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
    -158,  -158,   145,  -158,  -158,  -158,  -158,     1,    82,   152,
    -158,  -158,   111,   -61,     1,  -158,   155,   157,   -22,    67,
    -158,  -158,   -38,  -158,    24,   219,     7,  -158,   -14,   102,
    -158,   227,   180,   180,   227,   198,     1,  -158,   113,   227,
     180,   180,   -84,   -84,    34,    34,    34,  -158,    50,   -53,
    -158,   146,    53,   123,  -158,    62,   172,   165,    95,  -158,
    -158,  -103,    45,  -158,  -158,    47,    -1,  -158,  -158,  -158,
    -158,  -158,    96,    71,  -158,   227,   106,   145,  -158,  -158,
    -158,  -158,  -158,   152,   105,    -9,   -51,  -158,    19,  -158,
     199,   106,     7,  -158,    72,     5,     1,  -158,  -158,  -158,
     155,  -158,  -158,  -158,  -158,  -158,  -158,   172,  -158,  -158,
     -47,  -158,   251,    -1,   -38,  -158,    90,   -37,  -158,  -158,
    -158,   106,  -158,    77,     1,  -158,     7,   165,  -158,  -158
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   129,     3,     6,     7,    12,     9,    10,     8,    11,
       5,    38,    39,    55,   113,    29,    19,     0,     0,     0,
      95,    99,   100,   102,     0,     0,   103,     0,     0,    48,
      62,   118,    68,    69,    70,    77,    71,    72,    74,    97,
      98,   101,    73,     0,   112,     0,     0,     0,     1,   128,
       2,    45,    46,     0,    44,     0,    61,     0,     0,     0,
      51,    14,     0,    33,     0,     0,    76,    75,     0,     0,
      51,     0,   116,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     117,    66,     0,    27,    41,    40,     4,     0,    55,     0,
      42,   114,     0,     0,     0,    28,     0,     0,     0,     0,
      96,    67,     0,    49,   104,   106,   118,   108,   124,    53,
      63,    89,    92,    93,    86,    84,     0,   115,    85,    88,
      90,    91,    79,    78,    81,    80,    82,    83,     0,    51,
      35,    58,    61,    59,    18,     0,     0,    50,     0,    15,
     130,     0,     0,    31,    94,     0,     0,   111,   120,   121,
     123,   122,     0,     0,    47,    87,     0,     0,    34,    57,
      56,    54,    43,     0,     0,     0,     0,    20,     0,    32,
       0,     0,     0,   109,   105,     0,     0,    37,    36,    60,
       0,    24,    25,    26,    23,    22,    17,     0,    13,   131,
       0,    64,   107,     0,     0,   126,     0,    52,    16,    21,
      30,     0,   110,     0,     0,    65,     0,   127,   119,   125
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -158,  -158,  -158,   153,  -158,  -158,    13,  -158,  -158,  -158,
      11,  -158,  -158,  -158,  -158,  -158,  -158,  -158,  -158,    37,
    -108,   212,     0,  -158,   158,  -158,  -158,   -64,  -158,   137,
    -158,    94,    52,  -158,   166,   -34,  -158,  -158,  -158,  -158,
    -158,  -158,  -144,  -104,  -158,   -94,  -158,  -158,  -145,  -158,
      56,  -158,    12,  -157,   126,  -158,  -158,    58,  -158,  -158,
    -158
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    10,    11,    12,    13,    72,   158,    14,    69,   186,
     187,   205,    15,    16,    17,    18,   118,    19,   149,   150,
      20,    21,    22,    65,    23,    38,    80,   115,   174,    66,
     181,   110,    39,   210,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,   123,   124,   194,
     125,    53,   126,   100,   101,   127,   172,   128,   228,    60,
     161
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      76,    77,    24,    78,    30,    31,    32,    33,    24,    57,
      82,     6,   159,    25,   165,   153,   129,   201,   155,    54,
     114,   193,    24,    83,    84,    85,    26,     6,   202,    27,
      28,   189,   197,   162,   190,   212,    29,    55,    61,    70,
     119,    73,    96,    97,    98,    99,    58,   211,   168,   131,
     132,   133,   134,   135,   169,   170,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,    62,   103,   222,   229,
     171,    59,   156,   151,    83,    84,    85,   225,    67,    68,
     157,   113,   203,   206,   177,   178,   207,   220,    71,   199,
     221,    74,   204,    75,    63,     9,   159,    79,    64,   102,
      81,    86,   175,    82,    34,   104,   223,    83,    84,    85,
      31,    32,    33,    87,   105,    89,   109,   107,   163,    88,
      83,    84,    85,     6,   111,   112,    35,    36,   114,   116,
     117,    90,   122,    91,    37,    92,    93,   120,   214,     1,
      94,    95,    96,    97,    98,    99,   136,   137,   148,    63,
     121,   154,    86,    83,    84,    85,     2,     3,    33,    31,
     160,   166,   173,    99,    87,     4,     5,   176,   184,   183,
      88,     6,    83,    84,    85,   185,     7,   188,   191,   195,
     196,   192,    90,   200,    91,    86,    92,    93,  -127,  -127,
     227,    94,    95,    96,    97,    98,    99,    87,    86,     8,
     208,   164,   209,    88,   224,    83,    84,    85,    89,   213,
      87,   226,   106,   218,   198,    90,    88,    91,   219,    92,
      93,    56,   179,   108,    94,    95,    96,    97,    98,    99,
      91,    86,    92,    93,    83,    84,    85,    94,    95,    96,
      97,    98,    99,    87,   180,   152,   182,   130,   217,    88,
      86,   215,   167,   216,     0,     9,     0,     0,     0,     0,
       0,    90,    87,    91,     0,    92,    93,     0,    88,     0,
      94,    95,    96,    97,    98,    99,     0,     0,     0,     0,
      90,  -126,    91,    86,    92,    93,     0,  -126,  -126,    94,
      95,    96,    97,    98,    99,     0,     0,     0,     0,  -127,
    -127,    88,  -126,  -126,    94,    95,    96,    97,    98,    99,
       0,     0,    86,  -125,     0,    91,     0,    92,    93,  -125,
    -125,     0,    94,    95,    96,    97,    98,    99,     0,     0,
       0,     0,     0,     0,  -125,  -125,     0,     0,     0,     0,
       0,     0,     0,     0,    91,     0,    92,    93,     0,     0,
       0,    94,    95,    96,    97,    98,    99
};

static const yytype_int16 yycheck[] =
{
      34,    35,     3,    37,     3,     4,     5,     6,     3,     9,
       3,    49,   116,     1,   122,   109,    80,    26,    79,     7,
      73,   166,     3,     7,     8,     9,    70,    49,    37,    79,
      79,   134,   176,    55,   137,   192,    82,    70,    11,    27,
      74,    29,   126,   127,   128,   129,     0,   191,    62,    83,
      84,    85,    86,    87,    68,    69,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,    39,    55,   213,   226,
      84,   136,   133,   107,     7,     8,     9,   221,   135,   110,
     114,    69,    91,   134,   137,   149,   137,   134,    99,   183,
     137,   133,   101,   135,    67,   133,   200,    79,    71,   105,
     137,    85,   136,     3,   103,   134,   214,     7,     8,     9,
       4,     5,     6,    97,   134,   108,    63,   109,   118,   103,
       7,     8,     9,    49,     3,   103,   125,   126,    73,    78,
     133,   115,   133,   117,   133,   119,   120,     3,   133,    17,
     124,   125,   126,   127,   128,   129,    85,     3,     3,    67,
     134,    40,    85,     7,     8,     9,    34,    35,     6,     4,
       3,   137,    60,   129,    97,    43,    44,   117,   106,    46,
     103,    49,     7,     8,     9,     3,    54,    82,   133,    83,
     109,   134,   115,    78,   117,    85,   119,   120,     8,     9,
     224,   124,   125,   126,   127,   128,   129,    97,    85,    77,
     188,   134,     3,   103,   114,     7,     8,     9,   108,   137,
      97,   134,    59,   200,   177,   115,   103,   117,   207,   119,
     120,     9,    76,    65,   124,   125,   126,   127,   128,   129,
     117,    85,   119,   120,     7,     8,     9,   124,   125,   126,
     127,   128,   129,    97,    98,   108,   152,    81,   196,   103,
      85,   195,   126,   195,    -1,   133,    -1,    -1,    -1,    -1,
      -1,   115,    97,   117,    -1,   119,   120,    -1,   103,    -1,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
     115,    62,   117,    85,   119,   120,    -1,    68,    69,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,   119,
     120,   103,    83,    84,   124,   125,   126,   127,   128,   129,
      -1,    -1,    85,    62,    -1,   117,    -1,   119,   120,    68,
      69,    -1,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   117,    -1,   119,   120,    -1,    -1,
      -1,   124,   125,   126,   127,   128,   129
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    17,    34,    35,    43,    44,    49,    54,    77,   133,
     139,   140,   141,   142,   145,   150,   151,   152,   153,   155,
     158,   159,   160,   162,     3,   190,    70,    79,    79,    82,
       3,     4,     5,     6,   103,   125,   126,   133,   163,   170,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   189,   190,    70,   159,   160,     0,   136,
     197,    11,    39,    67,    71,   161,   167,   135,   110,   146,
     190,    99,   143,   190,   133,   135,   173,   173,   173,    79,
     164,   137,     3,     7,     8,     9,    85,    97,   103,   108,
     115,   117,   119,   120,   124,   125,   126,   127,   128,   129,
     191,   192,   105,   190,   134,   134,   141,   109,   162,    63,
     169,     3,   103,   190,    73,   165,    78,   133,   154,   173,
       3,   134,   133,   185,   186,   188,   190,   193,   195,   165,
     172,   173,   173,   173,   173,   173,    85,     3,   173,   173,
     173,   173,   173,   173,   173,   173,   173,   173,     3,   156,
     157,   173,   167,   183,    40,    79,   133,   173,   144,   181,
       3,   198,    55,   160,   134,   158,   137,   192,    62,    68,
      69,    84,   194,    60,   166,   173,   117,   137,   165,    76,
      98,   168,   169,    46,   106,     3,   147,   148,    82,   134,
     137,   133,   134,   186,   187,    83,   109,   180,   157,   183,
      78,    26,    37,    91,   101,   149,   134,   137,   190,     3,
     171,   180,   191,   137,   133,   188,   195,   170,   144,   148,
     134,   137,   186,   158,   114,   180,   134,   173,   196,   191
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   138,   139,   140,   140,   141,   141,   141,   141,   141,
     141,   141,   141,   142,   143,   144,   145,   145,   146,   146,
     147,   147,   148,   149,   149,   149,   149,   150,   151,   152,
     153,   153,   154,   154,   155,   156,   156,   157,   158,   158,
     159,   159,   160,   160,   161,   161,   161,   162,   163,   164,
     165,   165,   166,   166,   167,   167,   168,   168,   168,   169,
     169,   169,   170,   170,   171,   171,   172,   173,   173,   173,
     173,   173,   174,   174,   174,   175,   175,   176,   176,   176,
     176,   176,   176,   176,   176,   176,   176,   176,   177,   177,
     177,   177,   177,   177,   178,   179,   179,   180,   180,   181,
     182,   182,   183,   184,   185,   185,   186,   186,   186,   187,
     187,   188,   189,   190,   190,   191,   191,   192,   192,   193,
     194,   194,   194,   194,   194,   195,   195,   196,   197,   197,
     198,   198
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     7,     1,     1,     8,     7,     3,     0,
       1,     3,     2,     1,     1,     1,     1,     3,     4,     2,
       8,     5,     3,     0,     5,     1,     3,     3,     1,     1,
       3,     3,     3,     5,     1,     1,     1,     5,     1,     2,
       2,     0,     3,     0,     4,     0,     1,     1,     0,     2,
       4,     0,     1,     3,     1,     3,     2,     3,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     4,     3,     3,
       3,     3,     3,     3,     4,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     4,     1,     1,
       3,     2,     1,     1,     3,     2,     1,     1,     0,     6,
       1,     1,     1,     1,     0,     4,     1,     1,     1,     0,
       1,     3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (result, scanner, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if HSQL_DEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, result, scanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, hsql::SQLStatementList** result, yyscan_t scanner)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (result);
  YYUSE (scanner);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, hsql::SQLStatementList** result, yyscan_t scanner)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, result, scanner);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, hsql::SQLStatementList** result, yyscan_t scanner)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              , result, scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, result, scanner); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !HSQL_DEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !HSQL_DEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, hsql::SQLStatementList** result, yyscan_t scanner)
{
  YYUSE (yyvaluep);
  YYUSE (result);
  YYUSE (scanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (hsql::SQLStatementList** result, yyscan_t scanner)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, scanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 193 "bison_parser.y" /* yacc.c:1646  */
    { *result = (yyvsp[-1].stmt_list); }
#line 1651 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 198 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = new SQLStatementList((yyvsp[0].statement)); }
#line 1657 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 199 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].stmt_list)->push_back((yyvsp[0].statement)); (yyval.stmt_list) = (yyvsp[-2].stmt_list); }
#line 1663 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 203 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].select_stmt); }
#line 1669 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 204 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].import_stmt); }
#line 1675 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 205 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].create_stmt); }
#line 1681 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 206 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].insert_stmt); }
#line 1687 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 207 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].delete_stmt); }
#line 1693 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 208 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].delete_stmt); }
#line 1699 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 209 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].update_stmt); }
#line 1705 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 210 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].drop_stmt); }
#line 1711 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 219 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.import_stmt) = new ImportStatement((ImportStatement::ImportType) (yyvsp[-4].uval));
			(yyval.import_stmt)->file_path = (yyvsp[-2].sval);
			(yyval.import_stmt)->table_name = (yyvsp[0].sval);
		}
#line 1721 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 227 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = ImportStatement::kImportCSV; }
#line 1727 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 231 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.sval) = (yyvsp[0].expr)->name; }
#line 1733 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 241 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.create_stmt) = new CreateStatement(CreateStatement::kTableFromTbl);
			(yyval.create_stmt)->if_not_exists = (yyvsp[-5].bval);
			(yyval.create_stmt)->table_name = (yyvsp[-4].sval);
			(yyval.create_stmt)->file_path = (yyvsp[0].sval);
		}
#line 1744 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 247 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.create_stmt) = new CreateStatement(CreateStatement::kTable);
			(yyval.create_stmt)->if_not_exists = (yyvsp[-4].bval);
			(yyval.create_stmt)->table_name = (yyvsp[-3].sval);
			(yyval.create_stmt)->columns = (yyvsp[-1].column_list_t);
		}
#line 1755 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 256 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.bval) = true; }
#line 1761 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 257 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.bval) = false; }
#line 1767 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 261 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.column_list_t) = new List<ColumnDefinition*>((yyvsp[0].column_t)); }
#line 1773 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 262 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].column_list_t)->push_back((yyvsp[0].column_t)); (yyval.column_list_t) = (yyvsp[-2].column_list_t); }
#line 1779 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 266 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.column_t) = new ColumnDefinition((yyvsp[-1].sval), (ColumnDefinition::DataType) (yyvsp[0].uval));
		}
#line 1787 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 273 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = ColumnDefinition::INT; }
#line 1793 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 274 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = ColumnDefinition::INT; }
#line 1799 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 275 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = ColumnDefinition::DOUBLE; }
#line 1805 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 276 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = ColumnDefinition::TEXT; }
#line 1811 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 285 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.drop_stmt) = new DropStatement(DropStatement::kTable);
			(yyval.drop_stmt)->name = (yyvsp[0].sval);
		}
#line 1820 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 296 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.delete_stmt) = new DeleteStatement();
			(yyval.delete_stmt)->table_name = (yyvsp[-1].sval);
			(yyval.delete_stmt)->expr = (yyvsp[0].expr);
		}
#line 1830 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 304 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.delete_stmt) = new DeleteStatement();
			(yyval.delete_stmt)->table_name = (yyvsp[0].sval);
		}
#line 1839 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 316 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.insert_stmt) = new InsertStatement(InsertStatement::kInsertValues);
			(yyval.insert_stmt)->table_name = (yyvsp[-5].sval);
			(yyval.insert_stmt)->columns = (yyvsp[-4].slist);
			(yyval.insert_stmt)->values = (yyvsp[-1].expr_list);
		}
#line 1850 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 322 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.insert_stmt) = new InsertStatement(InsertStatement::kInsertSelect);
			(yyval.insert_stmt)->table_name = (yyvsp[-2].sval);
			(yyval.insert_stmt)->columns = (yyvsp[-1].slist);
			(yyval.insert_stmt)->select = (yyvsp[0].select_stmt);
		}
#line 1861 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 332 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.slist) = (yyvsp[-1].slist); }
#line 1867 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 333 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.slist) = NULL; }
#line 1873 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 343 "bison_parser.y" /* yacc.c:1646  */
    {
		(yyval.update_stmt) = new UpdateStatement();
		(yyval.update_stmt)->table = (yyvsp[-3].table);
		(yyval.update_stmt)->updates = (yyvsp[-1].update_list_t);
		(yyval.update_stmt)->where = (yyvsp[0].expr);
	}
#line 1884 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 352 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.update_list_t) = new List<UpdateClause*>((yyvsp[0].update_t)); }
#line 1890 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 353 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].update_list_t)->push_back((yyvsp[0].update_t)); (yyval.update_list_t) = (yyvsp[-2].update_list_t); }
#line 1896 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 357 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.update_t) = new UpdateClause();
			(yyval.update_t)->column = (yyvsp[-2].sval);
			(yyval.update_t)->value = (yyvsp[0].expr);
		}
#line 1906 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 374 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.select_stmt) = (yyvsp[-1].select_stmt); }
#line 1912 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 375 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.select_stmt) = (yyvsp[-1].select_stmt); }
#line 1918 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 379 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.select_stmt) = (yyvsp[-2].select_stmt);
			(yyval.select_stmt)->order = (yyvsp[-1].order);
			(yyval.select_stmt)->limit = (yyvsp[0].limit);
		}
#line 1928 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 43:
#line 384 "bison_parser.y" /* yacc.c:1646  */
    {
			// TODO: allow multiple unions (through linked list)
			// TODO: capture type of set_operator
			// TODO: might overwrite order and limit of first select here
			(yyval.select_stmt) = (yyvsp[-4].select_stmt);
			(yyval.select_stmt)->union_select = (yyvsp[-2].select_stmt);
			(yyval.select_stmt)->order = (yyvsp[-1].order);
			(yyval.select_stmt)->limit = (yyvsp[0].limit);
		}
#line 1942 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 402 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.select_stmt) = new SelectStatement();
			(yyval.select_stmt)->select_list = (yyvsp[-3].expr_list);
			(yyval.select_stmt)->from_table = (yyvsp[-2].table);
			(yyval.select_stmt)->where_clause = (yyvsp[-1].expr);
			(yyval.select_stmt)->group_by = (yyvsp[0].expr_list);
		}
#line 1954 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 49:
#line 418 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.table) = (yyvsp[0].table); }
#line 1960 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 50:
#line 423 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 1966 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 51:
#line 424 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = NULL; }
#line 1972 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 52:
#line 429 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = (yyvsp[0].expr_list); }
#line 1978 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 53:
#line 430 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = NULL; }
#line 1984 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 54:
#line 435 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.order) = new OrderDescription((yyvsp[0].order_type), (yyvsp[-1].expr)); }
#line 1990 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 55:
#line 436 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.order) = NULL; }
#line 1996 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 440 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.order_type) = kOrderAsc; }
#line 2002 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 441 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.order_type) = kOrderDesc; }
#line 2008 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 442 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.order_type) = kOrderAsc; }
#line 2014 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 447 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.limit) = new LimitDescription((yyvsp[0].expr)->ival, kNoOffset); delete (yyvsp[0].expr); }
#line 2020 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 448 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.limit) = new LimitDescription((yyvsp[-2].expr)->ival, (yyvsp[0].expr)->ival); delete (yyvsp[-2].expr); delete (yyvsp[0].expr); }
#line 2026 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 449 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.limit) = NULL; }
#line 2032 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 456 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = new List<Expr*>((yyvsp[0].expr)); }
#line 2038 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 457 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].expr_list)->push_back((yyvsp[0].expr)); (yyval.expr_list) = (yyvsp[-2].expr_list); }
#line 2044 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 461 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = new List<Expr*>((yyvsp[0].expr)); }
#line 2050 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 65:
#line 462 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].expr_list)->push_back((yyvsp[0].expr)); (yyval.expr_list) = (yyvsp[-2].expr_list); }
#line 2056 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 466 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.expr) = (yyvsp[-1].expr);
			(yyval.expr)->alias = (yyvsp[0].sval);
		}
#line 2065 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 473 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); }
#line 2071 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 487 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpUnary(Expr::UMINUS, (yyvsp[0].expr)); }
#line 2077 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 488 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpUnary(Expr::NOT, (yyvsp[0].expr)); }
#line 2083 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 78:
#line 493 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '-', (yyvsp[0].expr)); }
#line 2089 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 79:
#line 494 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '+', (yyvsp[0].expr)); }
#line 2095 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 80:
#line 495 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '/', (yyvsp[0].expr)); }
#line 2101 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 81:
#line 496 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '*', (yyvsp[0].expr)); }
#line 2107 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 82:
#line 497 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '%', (yyvsp[0].expr)); }
#line 2113 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 83:
#line 498 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '^', (yyvsp[0].expr)); }
#line 2119 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 499 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::AND, (yyvsp[0].expr)); }
#line 2125 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 85:
#line 500 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::OR, (yyvsp[0].expr)); }
#line 2131 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 86:
#line 501 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::LIKE, (yyvsp[0].expr)); }
#line 2137 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 87:
#line 502 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-3].expr), Expr::NOT_LIKE, (yyvsp[0].expr)); }
#line 2143 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 507 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '=', (yyvsp[0].expr)); }
#line 2149 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 89:
#line 508 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::NOT_EQUALS, (yyvsp[0].expr)); }
#line 2155 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 509 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '<', (yyvsp[0].expr)); }
#line 2161 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 91:
#line 510 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '>', (yyvsp[0].expr)); }
#line 2167 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 511 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::LESS_EQ, (yyvsp[0].expr)); }
#line 2173 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 93:
#line 512 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::GREATER_EQ, (yyvsp[0].expr)); }
#line 2179 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 94:
#line 516 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeFunctionRef((yyvsp[-3].sval), (yyvsp[-1].expr)); }
#line 2185 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 95:
#line 520 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeColumnRef((yyvsp[0].sval)); }
#line 2191 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 96:
#line 521 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeColumnRef((yyvsp[-2].sval), (yyvsp[0].sval)); }
#line 2197 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 99:
#line 530 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeLiteral((yyvsp[0].sval)); }
#line 2203 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 535 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeLiteral((yyvsp[0].fval)); }
#line 2209 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 540 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeLiteral((yyvsp[0].ival)); }
#line 2215 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 544 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = new Expr(kExprStar); }
#line 2221 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 553 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyvsp[0].table_list)->push_back((yyvsp[-2].table));
			auto tbl = new TableRef(kTableCrossProduct);
			tbl->list = (yyvsp[0].table_list);
			(yyval.table) = tbl;
		}
#line 2232 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 564 "bison_parser.y" /* yacc.c:1646  */
    {
			auto tbl = new TableRef(kTableSelect);
			tbl->select = (yyvsp[-2].select_stmt);
			tbl->alias = (yyvsp[0].sval);
			(yyval.table) = tbl;
		}
#line 2243 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 575 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.table_list) = new List<TableRef*>((yyvsp[0].table)); }
#line 2249 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 576 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].table_list)->push_back((yyvsp[0].table)); (yyval.table_list) = (yyvsp[-2].table_list); }
#line 2255 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 581 "bison_parser.y" /* yacc.c:1646  */
    {
			auto tbl = new TableRef(kTableName);
			tbl->name = (yyvsp[-1].sval);
			tbl->alias = (yyvsp[0].sval);
			(yyval.table) = tbl;
		}
#line 2266 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 591 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.table) = new TableRef(kTableName);
			(yyval.table)->name = (yyvsp[0].sval);
		}
#line 2275 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 605 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.sval) = (yyvsp[0].sval); }
#line 2281 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 611 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.sval) = NULL; }
#line 2287 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 620 "bison_parser.y" /* yacc.c:1646  */
    { 
			(yyval.table) = new TableRef(kTableJoin);
			(yyval.table)->join = new JoinDefinition();
			(yyval.table)->join->type = (JoinType) (yyvsp[-4].uval);
			(yyval.table)->join->left = (yyvsp[-5].table);
			(yyval.table)->join->right = (yyvsp[-2].table);
			(yyval.table)->join->condition = (yyvsp[0].expr);
		}
#line 2300 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 631 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = kJoinInner; }
#line 2306 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 632 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = kJoinOuter; }
#line 2312 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 633 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = kJoinLeft; }
#line 2318 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 634 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = kJoinRight; }
#line 2324 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 635 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = kJoinInner; }
#line 2330 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 641 "bison_parser.y" /* yacc.c:1646  */
    {
			auto tbl = new TableRef(kTableSelect);
			tbl->select = (yyvsp[-2].select_stmt);
			tbl->alias = (yyvsp[0].sval);
			(yyval.table) = tbl;
		}
#line 2341 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 666 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.slist) = new List<char*>((yyvsp[0].sval)); }
#line 2347 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 667 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].slist)->push_back((yyvsp[0].sval)); (yyval.slist) = (yyvsp[-2].slist); }
#line 2353 "bison_parser.cpp" /* yacc.c:1646  */
    break;


#line 2357 "bison_parser.cpp" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (result, scanner, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (result, scanner, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, result, scanner);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, result, scanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (result, scanner, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, result, scanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, result, scanner);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 670 "bison_parser.y" /* yacc.c:1906  */

/*********************************
 ** Section 4: Additional C code
 *********************************/

/* empty */

