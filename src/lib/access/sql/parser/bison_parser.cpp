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
#define YYLTYPE         HSQL_LTYPE
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

int yyerror(YYLTYPE* llocp, SQLStatementList** result, yyscan_t scanner, const char *msg) {

	SQLStatementList* list = new SQLStatementList();
	list->isValid = false;
	list->parser_msg = strdup(msg);
	list->error_line = llocp->first_line;
	list->error_col = llocp->first_column;

	*result = list;
	return 0;
}




#line 111 "bison_parser.cpp" /* yacc.c:339  */

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
#line 43 "bison_parser.y" /* yacc.c:355  */

// %code requires block	
#include "parser_typedef.h"

// Auto update column and line number
#define YY_USER_ACTION \
    yylloc->first_line = yylloc->last_line; \
    yylloc->first_column = yylloc->last_column; \
    for(int i = 0; yytext[i] != '\0'; i++) { \
        if(yytext[i] == '\n') { \
            yylloc->last_line++; \
            yylloc->last_column = 0; \
        } \
        else { \
            yylloc->last_column++; \
        } \
    }

#line 168 "bison_parser.cpp" /* yacc.c:355  */

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
    SQL_DEALLOCATE = 265,
    SQL_PARAMETERS = 266,
    SQL_INTERSECT = 267,
    SQL_TEMPORARY = 268,
    SQL_TIMESTAMP = 269,
    SQL_DISTINCT = 270,
    SQL_NVARCHAR = 271,
    SQL_RESTRICT = 272,
    SQL_TRUNCATE = 273,
    SQL_ANALYZE = 274,
    SQL_BETWEEN = 275,
    SQL_CASCADE = 276,
    SQL_COLUMNS = 277,
    SQL_CONTROL = 278,
    SQL_DEFAULT = 279,
    SQL_EXECUTE = 280,
    SQL_EXPLAIN = 281,
    SQL_HISTORY = 282,
    SQL_INTEGER = 283,
    SQL_NATURAL = 284,
    SQL_PREPARE = 285,
    SQL_PRIMARY = 286,
    SQL_SCHEMAS = 287,
    SQL_SPATIAL = 288,
    SQL_VIRTUAL = 289,
    SQL_BEFORE = 290,
    SQL_COLUMN = 291,
    SQL_CREATE = 292,
    SQL_DELETE = 293,
    SQL_DIRECT = 294,
    SQL_DOUBLE = 295,
    SQL_ESCAPE = 296,
    SQL_EXCEPT = 297,
    SQL_EXISTS = 298,
    SQL_GLOBAL = 299,
    SQL_HAVING = 300,
    SQL_IMPORT = 301,
    SQL_INSERT = 302,
    SQL_ISNULL = 303,
    SQL_OFFSET = 304,
    SQL_RENAME = 305,
    SQL_SCHEMA = 306,
    SQL_SELECT = 307,
    SQL_SORTED = 308,
    SQL_TABLES = 309,
    SQL_UNIQUE = 310,
    SQL_UNLOAD = 311,
    SQL_UPDATE = 312,
    SQL_VALUES = 313,
    SQL_AFTER = 314,
    SQL_ALTER = 315,
    SQL_CROSS = 316,
    SQL_DELTA = 317,
    SQL_GROUP = 318,
    SQL_INDEX = 319,
    SQL_INNER = 320,
    SQL_LIMIT = 321,
    SQL_LOCAL = 322,
    SQL_MERGE = 323,
    SQL_MINUS = 324,
    SQL_ORDER = 325,
    SQL_OUTER = 326,
    SQL_RIGHT = 327,
    SQL_TABLE = 328,
    SQL_UNION = 329,
    SQL_USING = 330,
    SQL_WHERE = 331,
    SQL_CALL = 332,
    SQL_DATE = 333,
    SQL_DESC = 334,
    SQL_DROP = 335,
    SQL_FILE = 336,
    SQL_FROM = 337,
    SQL_FULL = 338,
    SQL_HASH = 339,
    SQL_HINT = 340,
    SQL_INTO = 341,
    SQL_JOIN = 342,
    SQL_LEFT = 343,
    SQL_LIKE = 344,
    SQL_LOAD = 345,
    SQL_NULL = 346,
    SQL_PART = 347,
    SQL_PLAN = 348,
    SQL_SHOW = 349,
    SQL_TEXT = 350,
    SQL_TIME = 351,
    SQL_VIEW = 352,
    SQL_WITH = 353,
    SQL_ADD = 354,
    SQL_ALL = 355,
    SQL_AND = 356,
    SQL_ASC = 357,
    SQL_CSV = 358,
    SQL_FOR = 359,
    SQL_INT = 360,
    SQL_KEY = 361,
    SQL_NOT = 362,
    SQL_OFF = 363,
    SQL_SET = 364,
    SQL_TBL = 365,
    SQL_TOP = 366,
    SQL_AS = 367,
    SQL_BY = 368,
    SQL_IF = 369,
    SQL_IN = 370,
    SQL_IS = 371,
    SQL_OF = 372,
    SQL_ON = 373,
    SQL_OR = 374,
    SQL_TO = 375,
    SQL_EQUALS = 376,
    SQL_LESS = 377,
    SQL_GREATER = 378,
    SQL_NOTNULL = 379,
    SQL_UMINUS = 380
  };
#endif

/* Value type.  */
#if ! defined HSQL_STYPE && ! defined HSQL_STYPE_IS_DECLARED
typedef union HSQL_STYPE HSQL_STYPE;
union HSQL_STYPE
{
#line 97 "bison_parser.y" /* yacc.c:355  */

	double fval;
	int64_t ival;
	char* sval;
	uint uval;
	bool bval;

	hsql::SQLStatement* statement;
	hsql::SelectStatement* 	select_stmt;
	hsql::ImportStatement* 	import_stmt;
	hsql::CreateStatement* 	create_stmt;
	hsql::InsertStatement* 	insert_stmt;
	hsql::DeleteStatement* 	delete_stmt;
	hsql::UpdateStatement* 	update_stmt;
	hsql::DropStatement*   	drop_stmt;
	hsql::PrepareStatement* prep_stmt;
	hsql::ExecuteStatement* exec_stmt;

	hsql::TableRef* table;
	hsql::Expr* expr;
	hsql::OrderDescription* order;
	hsql::OrderType order_type;
	hsql::LimitDescription* limit;
	hsql::ColumnDefinition* column_t;
	hsql::GroupByDescription* group_t;
	hsql::UpdateClause* update_t;

	hsql::SQLStatementList* stmt_list;
	hsql::List<char*>* slist;
	hsql::List<hsql::Expr*>* expr_list;
	hsql::List<hsql::TableRef*>* table_list;
	hsql::List<hsql::ColumnDefinition*>* column_list_t;
	hsql::List<hsql::UpdateClause*>* update_list_t;

#line 341 "bison_parser.cpp" /* yacc.c:355  */
};
# define HSQL_STYPE_IS_TRIVIAL 1
# define HSQL_STYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined HSQL_LTYPE && ! defined HSQL_LTYPE_IS_DECLARED
typedef struct HSQL_LTYPE HSQL_LTYPE;
struct HSQL_LTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define HSQL_LTYPE_IS_DECLARED 1
# define HSQL_LTYPE_IS_TRIVIAL 1
#endif



int hsql_parse (hsql::SQLStatementList** result, yyscan_t scanner);

#endif /* !YY_HSQL_BISON_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 369 "bison_parser.cpp" /* yacc.c:358  */

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
         || (defined HSQL_LTYPE_IS_TRIVIAL && HSQL_LTYPE_IS_TRIVIAL \
             && defined HSQL_STYPE_IS_TRIVIAL && HSQL_STYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

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
#define YYFINAL  44
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   474

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  144
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  67
/* YYNRULES -- Number of rules.  */
#define YYNRULES  142
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  250

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   380

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   132,     2,     2,
     137,   138,   130,   128,   142,   129,   139,   131,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   141,   140,
     123,   121,   124,   143,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   135,     2,   136,   133,     2,     2,     2,     2,     2,
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
     115,   116,   117,   118,   119,   120,   122,   125,   126,   127,
     134
};

#if HSQL_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   223,   223,   228,   229,   233,   234,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   255,   263,   275,   283,
     287,   297,   303,   312,   313,   317,   318,   322,   329,   330,
     331,   332,   341,   352,   360,   372,   378,   388,   389,   399,
     408,   409,   413,   425,   426,   430,   431,   435,   440,   452,
     453,   454,   458,   469,   470,   474,   479,   484,   485,   490,
     495,   499,   500,   503,   504,   508,   509,   510,   515,   516,
     517,   524,   525,   529,   530,   534,   541,   542,   543,   544,
     545,   549,   550,   551,   555,   556,   560,   561,   562,   563,
     564,   565,   566,   567,   568,   569,   570,   575,   576,   577,
     578,   579,   580,   584,   588,   589,   593,   594,   595,   599,
     604,   605,   609,   613,   618,   626,   627,   637,   638,   644,
     649,   650,   655,   665,   673,   674,   679,   680,   684,   685,
     693,   705,   706,   707,   708,   709,   715,   721,   725,   734,
     735,   740,   741
};
#endif

#if HSQL_DEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENTIFIER", "STRING", "FLOATVAL",
  "INTVAL", "NOTEQUALS", "LESSEQ", "GREATEREQ", "DEALLOCATE", "PARAMETERS",
  "INTERSECT", "TEMPORARY", "TIMESTAMP", "DISTINCT", "NVARCHAR",
  "RESTRICT", "TRUNCATE", "ANALYZE", "BETWEEN", "CASCADE", "COLUMNS",
  "CONTROL", "DEFAULT", "EXECUTE", "EXPLAIN", "HISTORY", "INTEGER",
  "NATURAL", "PREPARE", "PRIMARY", "SCHEMAS", "SPATIAL", "VIRTUAL",
  "BEFORE", "COLUMN", "CREATE", "DELETE", "DIRECT", "DOUBLE", "ESCAPE",
  "EXCEPT", "EXISTS", "GLOBAL", "HAVING", "IMPORT", "INSERT", "ISNULL",
  "OFFSET", "RENAME", "SCHEMA", "SELECT", "SORTED", "TABLES", "UNIQUE",
  "UNLOAD", "UPDATE", "VALUES", "AFTER", "ALTER", "CROSS", "DELTA",
  "GROUP", "INDEX", "INNER", "LIMIT", "LOCAL", "MERGE", "MINUS", "ORDER",
  "OUTER", "RIGHT", "TABLE", "UNION", "USING", "WHERE", "CALL", "DATE",
  "DESC", "DROP", "FILE", "FROM", "FULL", "HASH", "HINT", "INTO", "JOIN",
  "LEFT", "LIKE", "LOAD", "NULL", "PART", "PLAN", "SHOW", "TEXT", "TIME",
  "VIEW", "WITH", "ADD", "ALL", "AND", "ASC", "CSV", "FOR", "INT", "KEY",
  "NOT", "OFF", "SET", "TBL", "TOP", "AS", "BY", "IF", "IN", "IS", "OF",
  "ON", "OR", "TO", "'='", "EQUALS", "'<'", "'>'", "LESS", "GREATER",
  "NOTNULL", "'+'", "'-'", "'*'", "'/'", "'%'", "'^'", "UMINUS", "'['",
  "']'", "'('", "')'", "'.'", "';'", "':'", "','", "'?'", "$accept",
  "input", "statement_list", "statement", "preparable_statement",
  "prepare_statement", "execute_statement", "import_statement",
  "import_file_type", "file_path", "create_statement", "opt_not_exists",
  "column_def_commalist", "column_def", "column_type", "drop_statement",
  "delete_statement", "truncate_statement", "insert_statement",
  "opt_column_list", "update_statement", "update_clause_commalist",
  "update_clause", "select_statement", "select_with_paren",
  "select_no_paren", "set_operator", "select_clause", "opt_distinct",
  "select_list", "from_clause", "opt_where", "opt_group", "opt_having",
  "opt_order", "opt_order_type", "opt_limit", "expr_list", "literal_list",
  "expr_alias", "expr", "scalar_expr", "unary_expr", "binary_expr",
  "comp_expr", "function_expr", "column_name", "literal", "string_literal",
  "num_literal", "int_literal", "star_expr", "placeholder_expr",
  "table_ref", "table_ref_atomic", "table_ref_commalist", "table_ref_name",
  "table_ref_name_no_alias", "table_name", "alias", "opt_alias",
  "join_clause", "opt_join_type", "join_table", "join_condition",
  "opt_semicolon", "ident_commalist", YY_NULLPTR
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
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,    61,   376,    60,    62,   377,   378,   379,    43,    45,
      42,    47,    37,    94,   380,    91,    93,    40,    41,    46,
      59,    58,    44,    63
};
# endif

#define YYPACT_NINF -205

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-205)))

#define YYTABLE_NINF -138

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-138)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     337,    26,    33,    68,   -16,    -7,    -6,    -2,    64,    26,
       7,   -34,    85,   -53,  -205,  -205,  -205,  -205,  -205,  -205,
    -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,    32,   -46,
    -205,   -43,   -44,   -19,    26,    -3,    26,  -205,     6,     2,
    -205,    26,   -31,   -30,  -205,   337,  -205,  -205,  -205,     5,
    -205,    69,    56,   125,     1,    52,    22,    26,    57,  -205,
      49,     3,   -89,  -205,  -205,  -205,     6,     6,  -205,     6,
    -205,    72,    -8,  -205,   107,  -205,  -205,  -205,  -205,  -205,
    -205,  -205,  -205,  -205,  -205,  -205,  -205,   143,  -205,  -205,
    -205,  -205,     6,    86,   149,  -205,  -205,  -110,  -205,  -205,
     115,   -59,     6,  -205,   156,   161,   -28,    64,   162,   240,
     111,    38,    11,    57,     6,  -205,     6,     6,     6,     6,
       6,    83,   170,     6,     6,     6,     6,     6,     6,     6,
       6,     6,     6,  -205,  -205,    53,   -50,  -205,   144,    56,
     130,  -205,     1,  -205,    70,   179,   177,    97,  -205,  -205,
    -100,    50,  -205,     6,  -205,  -205,   -34,  -205,    46,    54,
      12,  -205,   146,   127,  -205,   213,   129,   129,   213,   240,
       6,  -205,   192,   213,   129,   129,   111,   111,    58,    58,
      58,  -205,     1,   143,  -205,  -205,  -205,  -205,  -205,   149,
    -205,   128,    -9,   -99,  -205,    26,  -205,   189,     1,    74,
      75,    11,  -205,  -205,  -205,  -205,  -205,   123,   102,  -205,
     213,  -205,  -205,  -205,   156,  -205,  -205,  -205,  -205,  -205,
    -205,   179,  -205,  -205,   -69,  -205,    12,  -205,    82,    13,
       6,  -205,  -205,  -205,   199,    11,   -34,  -205,    98,   -25,
    -205,    87,     6,     6,  -205,    12,   177,  -205,   177,  -205
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,    54,     0,
       0,     0,     0,   140,     3,     5,     6,    15,     8,     9,
      14,    11,    12,    10,    13,     7,    43,    44,    64,   124,
      34,     0,     0,    24,     0,     0,     0,    53,     0,     0,
     123,     0,     0,     0,     1,   139,     2,    50,    51,     0,
      49,     0,    70,     0,     0,     0,     0,     0,    58,    19,
       0,    38,   104,   109,   110,   112,     0,     0,   113,     0,
     114,     0,    55,    71,   129,    77,    78,    79,    86,    80,
      81,    83,   106,   107,   111,    82,   108,     0,    32,    46,
      45,     4,     0,    64,     0,    47,   125,     0,    73,    16,
       0,     0,     0,    33,     0,     0,     0,    54,     0,    85,
      84,     0,     0,    58,     0,   127,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   128,    75,     0,    58,    40,    67,    70,
      68,    17,     0,    23,     0,     0,    57,     0,    20,   141,
       0,     0,    36,     0,   105,    76,     0,    56,   115,   117,
     129,   119,   135,    60,    72,    98,   101,   102,    95,    93,
       0,   126,    94,    97,    99,   100,    88,    87,    90,    89,
      91,    92,     0,     0,    39,    66,    65,    63,    48,     0,
      74,     0,     0,     0,    25,     0,    37,     0,     0,     0,
       0,     0,   122,   131,   132,   134,   133,     0,     0,    52,
      96,    42,    41,    69,     0,    29,    30,    31,    28,    27,
      22,     0,    18,   142,     0,   103,     0,   120,   116,     0,
       0,    21,    26,    35,   118,     0,     0,   137,     0,    62,
     121,     0,     0,     0,    59,     0,   138,   130,    61,   136
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -205,  -205,  -205,   182,   174,  -205,  -205,  -205,  -205,    18,
    -205,  -205,  -205,    29,  -205,  -205,  -205,  -205,  -205,  -205,
    -205,  -205,    71,  -135,   244,    14,  -205,   205,   172,  -205,
    -205,   -64,  -205,  -205,   176,  -205,   141,    55,    84,   169,
     -65,  -205,  -205,  -205,  -205,  -205,  -205,   -51,   -91,  -205,
     -77,  -205,  -205,  -205,  -167,  -205,    59,  -205,    -1,  -204,
     131,  -205,  -205,    60,  -205,  -205,  -205
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    12,    13,    14,    15,    16,    17,    18,    60,   147,
      19,    57,   193,   194,   219,    20,    21,    22,    23,   106,
      24,   136,   137,    25,    26,    27,    51,    28,    38,    71,
     113,   103,   209,   244,    52,   187,    95,    72,    97,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,   157,   158,   228,   159,    39,   160,   133,
     134,   161,   207,   162,   247,    46,   150
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      30,   109,   110,    98,   111,    63,    64,    65,    40,    62,
      63,    64,    65,   148,    29,   115,    29,   140,     8,   215,
     243,   200,   234,   144,     8,    43,   102,   138,   141,    29,
     151,   216,   142,    58,   227,    61,    31,   146,   196,   220,
      88,   249,   197,   221,    47,   116,   117,   118,   107,   163,
     108,   165,   166,   167,   168,   169,   101,    33,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   240,   233,
       1,    32,   184,   142,    48,    34,    35,     2,   145,    37,
      41,   116,   117,   118,    36,    44,   217,    45,   199,     4,
       5,   190,   183,    53,    54,    56,   218,    55,     6,     7,
      59,   241,    49,    11,     8,   210,    50,    89,    90,     9,
     115,    87,   213,    66,   116,   117,   118,   114,    92,  -137,
     152,     8,    94,   148,   122,  -137,  -137,   119,    96,   100,
     104,   211,    10,   102,   114,    67,    68,  -138,  -138,   120,
     105,  -137,  -137,    69,    70,   121,   135,    98,   156,    70,
     236,   116,   117,   118,   112,    65,    49,   123,   143,   124,
      63,   125,   126,   119,   149,   154,   127,   128,   129,   130,
     131,   132,   170,   171,   182,   120,   155,   246,   248,   189,
     191,   121,   192,   195,   116,   117,   118,   198,   201,    11,
     208,   132,   223,   123,   222,   124,   119,   125,   126,   116,
     117,   118,   127,   128,   129,   130,   131,   132,   120,   214,
     229,   203,   225,   226,   121,   230,   242,   204,   205,   122,
     116,   117,   118,   185,   235,   245,   123,    91,   124,    99,
     125,   126,   231,   119,   206,   127,   128,   129,   130,   131,
     132,   129,   130,   131,   132,   120,   186,   116,   117,   118,
     232,   121,  -138,  -138,   212,    42,    93,   127,   128,   129,
     130,   131,   132,   123,  -136,   124,   119,   125,   126,   139,
    -136,  -136,   127,   128,   129,   130,   131,   132,   120,   153,
     188,   119,   224,   164,   121,   239,  -136,  -136,   237,   238,
       0,   202,     0,   120,     0,     0,   123,     0,   124,   121,
     125,   126,   119,     0,     0,   127,   128,   129,   130,   131,
     132,     0,     0,   124,     0,   125,   126,     0,     0,     0,
     127,   128,   129,   130,   131,   132,     0,     0,     0,   119,
       0,     0,     0,     0,   124,     0,   125,   126,     0,     0,
       0,   127,   128,   129,   130,   131,   132,   121,     0,     0,
       0,     0,     0,     0,     0,     1,     0,     0,     0,     0,
       0,   124,     2,   125,   126,     0,     0,     3,   127,   128,
     129,   130,   131,   132,     4,     5,     0,     0,     0,     0,
       0,     0,     0,     6,     7,     0,     0,     0,     0,     8,
       0,     0,     0,     0,     9,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11
};

static const yytype_int16 yycheck[] =
{
       1,    66,    67,    54,    69,     4,     5,     6,     9,     3,
       4,     5,     6,   104,     3,     3,     3,    94,    52,    28,
      45,   156,   226,    82,    52,    11,    76,    92,   138,     3,
      58,    40,   142,    34,   201,    36,     3,   102,   138,   138,
      41,   245,   142,   142,    12,     7,     8,     9,   137,   113,
     139,   116,   117,   118,   119,   120,    57,    73,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   235,   138,
      18,     3,   136,   142,    42,    82,    82,    25,   137,    15,
      73,     7,     8,     9,    86,     0,    95,   140,   153,    37,
      38,   142,   142,   139,   137,   114,   105,   141,    46,    47,
     103,   236,    70,   137,    52,   170,    74,   138,   138,    57,
       3,   109,   189,   107,     7,     8,     9,   142,   113,    65,
     106,    52,    66,   214,   112,    71,    72,    89,     3,   107,
      81,   182,    80,    76,   142,   129,   130,     8,     9,   101,
     137,    87,    88,   137,   143,   107,     3,   198,   137,   143,
     137,     7,     8,     9,    82,     6,    70,   119,    43,   121,
       4,   123,   124,    89,     3,     3,   128,   129,   130,   131,
     132,   133,    89,     3,   121,   101,   138,   242,   243,    49,
     110,   107,     3,    86,     7,     8,     9,   137,   142,   137,
      63,   133,     3,   119,   195,   121,    89,   123,   124,     7,
       8,     9,   128,   129,   130,   131,   132,   133,   101,    81,
      87,    65,   138,   138,   107,   113,   118,    71,    72,   112,
       7,     8,     9,    79,   142,   138,   119,    45,   121,    55,
     123,   124,   214,    89,    88,   128,   129,   130,   131,   132,
     133,   130,   131,   132,   133,   101,   102,     7,     8,     9,
     221,   107,   123,   124,   183,    11,    51,   128,   129,   130,
     131,   132,   133,   119,    65,   121,    89,   123,   124,    93,
      71,    72,   128,   129,   130,   131,   132,   133,   101,   107,
     139,    89,   198,   114,   107,   230,    87,    88,   229,   229,
      -1,   160,    -1,   101,    -1,    -1,   119,    -1,   121,   107,
     123,   124,    89,    -1,    -1,   128,   129,   130,   131,   132,
     133,    -1,    -1,   121,    -1,   123,   124,    -1,    -1,    -1,
     128,   129,   130,   131,   132,   133,    -1,    -1,    -1,    89,
      -1,    -1,    -1,    -1,   121,    -1,   123,   124,    -1,    -1,
      -1,   128,   129,   130,   131,   132,   133,   107,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    18,    -1,    -1,    -1,    -1,
      -1,   121,    25,   123,   124,    -1,    -1,    30,   128,   129,
     130,   131,   132,   133,    37,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    18,    25,    30,    37,    38,    46,    47,    52,    57,
      80,   137,   145,   146,   147,   148,   149,   150,   151,   154,
     159,   160,   161,   162,   164,   167,   168,   169,   171,     3,
     202,     3,     3,    73,    82,    82,    86,    15,   172,   201,
     202,    73,   168,   169,     0,   140,   209,    12,    42,    70,
      74,   170,   178,   139,   137,   141,   114,   155,   202,   103,
     152,   202,     3,     4,     5,     6,   107,   129,   130,   137,
     143,   173,   181,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   109,   202,   138,
     138,   147,   113,   171,    66,   180,     3,   182,   191,   148,
     107,   202,    76,   175,    81,   137,   163,   137,   139,   184,
     184,   184,    82,   174,   142,     3,     7,     8,     9,    89,
     101,   107,   112,   119,   121,   123,   124,   128,   129,   130,
     131,   132,   133,   203,   204,     3,   165,   166,   184,   178,
     194,   138,   142,    43,    82,   137,   184,   153,   192,     3,
     210,    58,   169,   172,     3,   138,   137,   197,   198,   200,
     202,   205,   207,   175,   183,   184,   184,   184,   184,   184,
      89,     3,   184,   184,   184,   184,   184,   184,   184,   184,
     184,   184,   121,   142,   175,    79,   102,   179,   180,    49,
     191,   110,     3,   156,   157,    86,   138,   142,   137,   184,
     167,   142,   204,    65,    71,    72,    88,   206,    63,   176,
     184,   191,   166,   194,    81,    28,    40,    95,   105,   158,
     138,   142,   202,     3,   182,   138,   138,   198,   199,    87,
     113,   153,   157,   138,   203,   142,   137,   200,   207,   181,
     198,   167,   118,    45,   177,   138,   184,   208,   184,   203
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   144,   145,   146,   146,   147,   147,   148,   148,   148,
     148,   148,   148,   148,   148,   148,   149,   150,   151,   152,
     153,   154,   154,   155,   155,   156,   156,   157,   158,   158,
     158,   158,   159,   160,   161,   162,   162,   163,   163,   164,
     165,   165,   166,   167,   167,   168,   168,   169,   169,   170,
     170,   170,   171,   172,   172,   173,   174,   175,   175,   176,
     176,   177,   177,   178,   178,   179,   179,   179,   180,   180,
     180,   181,   181,   182,   182,   183,   184,   184,   184,   184,
     184,   185,   185,   185,   186,   186,   187,   187,   187,   187,
     187,   187,   187,   187,   187,   187,   187,   188,   188,   188,
     188,   188,   188,   189,   190,   190,   191,   191,   191,   192,
     193,   193,   194,   195,   196,   197,   197,   198,   198,   198,
     199,   199,   200,   201,   202,   202,   203,   203,   204,   204,
     205,   206,   206,   206,   206,   206,   207,   207,   208,   209,
     209,   210,   210
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     4,     5,     7,     1,
       1,     8,     7,     3,     0,     1,     3,     2,     1,     1,
       1,     1,     3,     4,     2,     8,     5,     3,     0,     5,
       1,     3,     3,     1,     1,     3,     3,     3,     5,     1,
       1,     1,     6,     1,     0,     1,     2,     2,     0,     4,
       0,     2,     0,     4,     0,     1,     1,     0,     2,     4,
       0,     1,     3,     1,     3,     2,     3,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     4,     3,     3,     3,
       3,     3,     3,     5,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     4,     1,
       1,     3,     2,     1,     1,     3,     2,     1,     1,     0,
       6,     1,     1,     1,     1,     0,     4,     1,     1,     1,
       0,     1,     3
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
      yyerror (&yylloc, result, scanner, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


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


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined HSQL_LTYPE_IS_TRIVIAL && HSQL_LTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location, result, scanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, hsql::SQLStatementList** result, yyscan_t scanner)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, hsql::SQLStatementList** result, yyscan_t scanner)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, result, scanner);
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, hsql::SQLStatementList** result, yyscan_t scanner)
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
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , result, scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, result, scanner); \
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, hsql::SQLStatementList** result, yyscan_t scanner)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
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

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined HSQL_LTYPE_IS_TRIVIAL && HSQL_LTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

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

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

/* User initialization code.  */
#line 76 "bison_parser.y" /* yacc.c:1429  */
{
	// Initialize
	yylloc.first_column = 0;
	yylloc.last_column = 0;
	yylloc.first_line = 0;
	yylloc.last_line = 0;
	yylloc.placeholder_id = 0;
}

#line 1644 "bison_parser.cpp" /* yacc.c:1429  */
  yylsp[0] = yylloc;
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
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);

        yyls = yyls1;
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
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

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
      yychar = yylex (&yylval, &yylloc, scanner);
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
  *++yylsp = yylloc;
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

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 223 "bison_parser.y" /* yacc.c:1646  */
    { *result = (yyvsp[-1].stmt_list); }
#line 1833 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 228 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = new SQLStatementList((yyvsp[0].statement)); }
#line 1839 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 229 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].stmt_list)->push_back((yyvsp[0].statement)); (yyval.stmt_list) = (yyvsp[-2].stmt_list); }
#line 1845 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 234 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].prep_stmt); }
#line 1851 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 239 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].select_stmt); }
#line 1857 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 240 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].import_stmt); }
#line 1863 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 241 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].create_stmt); }
#line 1869 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 242 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].insert_stmt); }
#line 1875 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 243 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].delete_stmt); }
#line 1881 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 244 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].delete_stmt); }
#line 1887 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 245 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].update_stmt); }
#line 1893 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 246 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].drop_stmt); }
#line 1899 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 247 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].exec_stmt); }
#line 1905 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 255 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.prep_stmt) = new PrepareStatement();
			(yyval.prep_stmt)->name = (yyvsp[-2].sval);
			(yyval.prep_stmt)->stmt = (yyvsp[0].statement);
		}
#line 1915 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 263 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.exec_stmt) = new ExecuteStatement();
			(yyval.exec_stmt)->name = (yyvsp[-3].sval);
			(yyval.exec_stmt)->parameters = (yyvsp[-1].expr_list);
		}
#line 1925 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 275 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.import_stmt) = new ImportStatement((ImportStatement::ImportType) (yyvsp[-4].uval));
			(yyval.import_stmt)->file_path = (yyvsp[-2].sval);
			(yyval.import_stmt)->table_name = (yyvsp[0].sval);
		}
#line 1935 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 283 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = ImportStatement::kImportCSV; }
#line 1941 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 287 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.sval) = (yyvsp[0].expr)->name; }
#line 1947 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 297 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.create_stmt) = new CreateStatement(CreateStatement::kTableFromTbl);
			(yyval.create_stmt)->if_not_exists = (yyvsp[-5].bval);
			(yyval.create_stmt)->table_name = (yyvsp[-4].sval);
			(yyval.create_stmt)->file_path = (yyvsp[0].sval);
		}
#line 1958 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 303 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.create_stmt) = new CreateStatement(CreateStatement::kTable);
			(yyval.create_stmt)->if_not_exists = (yyvsp[-4].bval);
			(yyval.create_stmt)->table_name = (yyvsp[-3].sval);
			(yyval.create_stmt)->columns = (yyvsp[-1].column_list_t);
		}
#line 1969 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 312 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.bval) = true; }
#line 1975 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 313 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.bval) = false; }
#line 1981 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 317 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.column_list_t) = new List<ColumnDefinition*>((yyvsp[0].column_t)); }
#line 1987 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 318 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].column_list_t)->push_back((yyvsp[0].column_t)); (yyval.column_list_t) = (yyvsp[-2].column_list_t); }
#line 1993 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 322 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.column_t) = new ColumnDefinition((yyvsp[-1].sval), (ColumnDefinition::DataType) (yyvsp[0].uval));
		}
#line 2001 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 329 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = ColumnDefinition::INT; }
#line 2007 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 330 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = ColumnDefinition::INT; }
#line 2013 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 331 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = ColumnDefinition::DOUBLE; }
#line 2019 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 332 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = ColumnDefinition::TEXT; }
#line 2025 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 341 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.drop_stmt) = new DropStatement(DropStatement::kTable);
			(yyval.drop_stmt)->name = (yyvsp[0].sval);
		}
#line 2034 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 352 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.delete_stmt) = new DeleteStatement();
			(yyval.delete_stmt)->table_name = (yyvsp[-1].sval);
			(yyval.delete_stmt)->expr = (yyvsp[0].expr);
		}
#line 2044 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 360 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.delete_stmt) = new DeleteStatement();
			(yyval.delete_stmt)->table_name = (yyvsp[0].sval);
		}
#line 2053 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 372 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.insert_stmt) = new InsertStatement(InsertStatement::kInsertValues);
			(yyval.insert_stmt)->table_name = (yyvsp[-5].sval);
			(yyval.insert_stmt)->columns = (yyvsp[-4].slist);
			(yyval.insert_stmt)->values = (yyvsp[-1].expr_list);
		}
#line 2064 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 378 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.insert_stmt) = new InsertStatement(InsertStatement::kInsertSelect);
			(yyval.insert_stmt)->table_name = (yyvsp[-2].sval);
			(yyval.insert_stmt)->columns = (yyvsp[-1].slist);
			(yyval.insert_stmt)->select = (yyvsp[0].select_stmt);
		}
#line 2075 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 388 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.slist) = (yyvsp[-1].slist); }
#line 2081 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 389 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.slist) = NULL; }
#line 2087 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 399 "bison_parser.y" /* yacc.c:1646  */
    {
		(yyval.update_stmt) = new UpdateStatement();
		(yyval.update_stmt)->table = (yyvsp[-3].table);
		(yyval.update_stmt)->updates = (yyvsp[-1].update_list_t);
		(yyval.update_stmt)->where = (yyvsp[0].expr);
	}
#line 2098 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 408 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.update_list_t) = new List<UpdateClause*>((yyvsp[0].update_t)); }
#line 2104 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 409 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].update_list_t)->push_back((yyvsp[0].update_t)); (yyval.update_list_t) = (yyvsp[-2].update_list_t); }
#line 2110 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 413 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.update_t) = new UpdateClause();
			(yyval.update_t)->column = (yyvsp[-2].sval);
			(yyval.update_t)->value = (yyvsp[0].expr);
		}
#line 2120 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 430 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.select_stmt) = (yyvsp[-1].select_stmt); }
#line 2126 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 46:
#line 431 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.select_stmt) = (yyvsp[-1].select_stmt); }
#line 2132 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 435 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.select_stmt) = (yyvsp[-2].select_stmt);
			(yyval.select_stmt)->order = (yyvsp[-1].order);
			(yyval.select_stmt)->limit = (yyvsp[0].limit);
		}
#line 2142 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 440 "bison_parser.y" /* yacc.c:1646  */
    {
			// TODO: allow multiple unions (through linked list)
			// TODO: capture type of set_operator
			// TODO: might overwrite order and limit of first select here
			(yyval.select_stmt) = (yyvsp[-4].select_stmt);
			(yyval.select_stmt)->union_select = (yyvsp[-2].select_stmt);
			(yyval.select_stmt)->order = (yyvsp[-1].order);
			(yyval.select_stmt)->limit = (yyvsp[0].limit);
		}
#line 2156 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 52:
#line 458 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.select_stmt) = new SelectStatement();
			(yyval.select_stmt)->select_distinct = (yyvsp[-4].bval);
			(yyval.select_stmt)->select_list = (yyvsp[-3].expr_list);
			(yyval.select_stmt)->from_table = (yyvsp[-2].table);
			(yyval.select_stmt)->where_clause = (yyvsp[-1].expr);
			(yyval.select_stmt)->group_by = (yyvsp[0].group_t);
		}
#line 2169 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 53:
#line 469 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.bval) = true; }
#line 2175 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 54:
#line 470 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.bval) = false; }
#line 2181 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 479 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.table) = (yyvsp[0].table); }
#line 2187 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 484 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 2193 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 485 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = NULL; }
#line 2199 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 490 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.group_t) = new GroupByDescription();
			(yyval.group_t)->columns = (yyvsp[-1].expr_list);
			(yyval.group_t)->having = (yyvsp[0].expr);
		}
#line 2209 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 495 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.group_t) = NULL; }
#line 2215 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 499 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 2221 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 500 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = NULL; }
#line 2227 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 503 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.order) = new OrderDescription((yyvsp[0].order_type), (yyvsp[-1].expr)); }
#line 2233 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 504 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.order) = NULL; }
#line 2239 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 65:
#line 508 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.order_type) = kOrderAsc; }
#line 2245 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 509 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.order_type) = kOrderDesc; }
#line 2251 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 510 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.order_type) = kOrderAsc; }
#line 2257 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 68:
#line 515 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.limit) = new LimitDescription((yyvsp[0].expr)->ival, kNoOffset); delete (yyvsp[0].expr); }
#line 2263 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 69:
#line 516 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.limit) = new LimitDescription((yyvsp[-2].expr)->ival, (yyvsp[0].expr)->ival); delete (yyvsp[-2].expr); delete (yyvsp[0].expr); }
#line 2269 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 70:
#line 517 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.limit) = NULL; }
#line 2275 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 71:
#line 524 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = new List<Expr*>((yyvsp[0].expr)); }
#line 2281 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 72:
#line 525 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].expr_list)->push_back((yyvsp[0].expr)); (yyval.expr_list) = (yyvsp[-2].expr_list); }
#line 2287 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 73:
#line 529 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = new List<Expr*>((yyvsp[0].expr)); }
#line 2293 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 74:
#line 530 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].expr_list)->push_back((yyvsp[0].expr)); (yyval.expr_list) = (yyvsp[-2].expr_list); }
#line 2299 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 534 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.expr) = (yyvsp[-1].expr);
			(yyval.expr)->alias = (yyvsp[0].sval);
		}
#line 2308 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 541 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); }
#line 2314 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 555 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpUnary(Expr::UMINUS, (yyvsp[0].expr)); }
#line 2320 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 85:
#line 556 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpUnary(Expr::NOT, (yyvsp[0].expr)); }
#line 2326 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 87:
#line 561 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '-', (yyvsp[0].expr)); }
#line 2332 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 562 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '+', (yyvsp[0].expr)); }
#line 2338 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 89:
#line 563 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '/', (yyvsp[0].expr)); }
#line 2344 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 564 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '*', (yyvsp[0].expr)); }
#line 2350 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 91:
#line 565 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '%', (yyvsp[0].expr)); }
#line 2356 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 566 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '^', (yyvsp[0].expr)); }
#line 2362 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 93:
#line 567 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::AND, (yyvsp[0].expr)); }
#line 2368 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 94:
#line 568 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::OR, (yyvsp[0].expr)); }
#line 2374 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 95:
#line 569 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::LIKE, (yyvsp[0].expr)); }
#line 2380 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 96:
#line 570 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-3].expr), Expr::NOT_LIKE, (yyvsp[0].expr)); }
#line 2386 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 97:
#line 575 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '=', (yyvsp[0].expr)); }
#line 2392 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 98:
#line 576 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::NOT_EQUALS, (yyvsp[0].expr)); }
#line 2398 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 99:
#line 577 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '<', (yyvsp[0].expr)); }
#line 2404 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 578 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '>', (yyvsp[0].expr)); }
#line 2410 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 101:
#line 579 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::LESS_EQ, (yyvsp[0].expr)); }
#line 2416 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 580 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::GREATER_EQ, (yyvsp[0].expr)); }
#line 2422 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 584 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeFunctionRef((yyvsp[-4].sval), (yyvsp[-1].expr), (yyvsp[-2].bval)); }
#line 2428 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 588 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeColumnRef((yyvsp[0].sval)); }
#line 2434 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 589 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeColumnRef((yyvsp[-2].sval), (yyvsp[0].sval)); }
#line 2440 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 599 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeLiteral((yyvsp[0].sval)); }
#line 2446 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 604 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeLiteral((yyvsp[0].fval)); }
#line 2452 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 609 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeLiteral((yyvsp[0].ival)); }
#line 2458 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 613 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = new Expr(kExprStar); }
#line 2464 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 618 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = new Expr(kExprPlaceholder); (yyval.expr)->ival = yylloc.placeholder_id++; }
#line 2470 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 627 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyvsp[0].table_list)->push_back((yyvsp[-2].table));
			auto tbl = new TableRef(kTableCrossProduct);
			tbl->list = (yyvsp[0].table_list);
			(yyval.table) = tbl;
		}
#line 2481 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 638 "bison_parser.y" /* yacc.c:1646  */
    {
			auto tbl = new TableRef(kTableSelect);
			tbl->select = (yyvsp[-2].select_stmt);
			tbl->alias = (yyvsp[0].sval);
			(yyval.table) = tbl;
		}
#line 2492 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 649 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.table_list) = new List<TableRef*>((yyvsp[0].table)); }
#line 2498 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 650 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].table_list)->push_back((yyvsp[0].table)); (yyval.table_list) = (yyvsp[-2].table_list); }
#line 2504 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 655 "bison_parser.y" /* yacc.c:1646  */
    {
			auto tbl = new TableRef(kTableName);
			tbl->name = (yyvsp[-1].sval);
			tbl->alias = (yyvsp[0].sval);
			(yyval.table) = tbl;
		}
#line 2515 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 665 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.table) = new TableRef(kTableName);
			(yyval.table)->name = (yyvsp[0].sval);
		}
#line 2524 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 679 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.sval) = (yyvsp[0].sval); }
#line 2530 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 685 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.sval) = NULL; }
#line 2536 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 694 "bison_parser.y" /* yacc.c:1646  */
    { 
			(yyval.table) = new TableRef(kTableJoin);
			(yyval.table)->join = new JoinDefinition();
			(yyval.table)->join->type = (JoinType) (yyvsp[-4].uval);
			(yyval.table)->join->left = (yyvsp[-5].table);
			(yyval.table)->join->right = (yyvsp[-2].table);
			(yyval.table)->join->condition = (yyvsp[0].expr);
		}
#line 2549 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 705 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = kJoinInner; }
#line 2555 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 706 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = kJoinOuter; }
#line 2561 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 707 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = kJoinLeft; }
#line 2567 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 708 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = kJoinRight; }
#line 2573 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 709 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = kJoinInner; }
#line 2579 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 715 "bison_parser.y" /* yacc.c:1646  */
    {
			auto tbl = new TableRef(kTableSelect);
			tbl->select = (yyvsp[-2].select_stmt);
			tbl->alias = (yyvsp[0].sval);
			(yyval.table) = tbl;
		}
#line 2590 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 740 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.slist) = new List<char*>((yyvsp[0].sval)); }
#line 2596 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 741 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].slist)->push_back((yyvsp[0].sval)); (yyval.slist) = (yyvsp[-2].slist); }
#line 2602 "bison_parser.cpp" /* yacc.c:1646  */
    break;


#line 2606 "bison_parser.cpp" /* yacc.c:1646  */
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
  *++yylsp = yyloc;

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
      yyerror (&yylloc, result, scanner, YY_("syntax error"));
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
        yyerror (&yylloc, result, scanner, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

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
                      yytoken, &yylval, &yylloc, result, scanner);
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

  yyerror_range[1] = yylsp[1-yylen];
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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp, result, scanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

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
  yyerror (&yylloc, result, scanner, YY_("memory exhausted"));
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
                  yytoken, &yylval, &yylloc, result, scanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, result, scanner);
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
#line 744 "bison_parser.y" /* yacc.c:1906  */

/*********************************
 ** Section 4: Additional C code
 *********************************/

/* empty */

