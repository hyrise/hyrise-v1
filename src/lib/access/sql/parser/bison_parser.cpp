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

int yyerror(StatementList** result, yyscan_t scanner, const char *msg) {

	StatementList* list = new StatementList();
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
#line 74 "bison_parser.y" /* yacc.c:355  */

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

#line 298 "bison_parser.cpp" /* yacc.c:355  */
};
# define HSQL_STYPE_IS_TRIVIAL 1
# define HSQL_STYPE_IS_DECLARED 1
#endif



int hsql_parse (hsql::StatementList** result, yyscan_t scanner);

#endif /* !YY_HSQL_BISON_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 312 "bison_parser.cpp" /* yacc.c:358  */

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
#define YYFINAL  44
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   355

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  122
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  51
/* YYNRULES -- Number of rules.  */
#define YYNRULES  111
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  193

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   360

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   112,     2,     2,
     117,   118,   110,   108,   121,   109,   119,   111,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   120,
     103,   101,   104,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   115,     2,   116,   113,     2,     2,     2,     2,     2,
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
      95,    96,    97,    98,    99,   100,   102,   105,   106,   107,
     114
};

#if HSQL_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   177,   177,   182,   183,   187,   188,   189,   190,   199,
     208,   212,   220,   230,   231,   239,   246,   257,   258,   266,
     267,   271,   272,   276,   281,   293,   294,   295,   299,   310,
     315,   320,   321,   326,   327,   332,   333,   337,   338,   339,
     344,   345,   346,   353,   354,   358,   359,   363,   370,   371,
     372,   373,   374,   378,   379,   380,   384,   385,   389,   390,
     391,   392,   393,   394,   395,   396,   397,   398,   399,   404,
     405,   406,   407,   408,   409,   413,   417,   418,   422,   423,
     427,   432,   433,   437,   441,   449,   450,   460,   461,   467,
     472,   473,   478,   487,   488,   493,   494,   498,   499,   507,
     519,   520,   521,   522,   523,   529,   535,   539,   548,   549,
     554,   555
};
#endif

#if HSQL_DEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENTIFIER", "STRING", "FLOAT", "INT",
  "NOTEQUALS", "LESSEQ", "GREATEREQ", "INTERSECT", "TEMPORARY", "DISTINCT",
  "RESTRICT", "TRUNCATE", "ANALYZE", "BETWEEN", "CASCADE", "COLUMNS",
  "CONTROL", "DEFAULT", "EXPLAIN", "HISTORY", "NATURAL", "PRIMARY",
  "SCHEMAS", "SPATIAL", "BEFORE", "COLUMN", "CREATE", "DELETE", "DIRECT",
  "ESCAPE", "EXCEPT", "EXISTS", "GLOBAL", "HAVING", "IMPORT", "INSERT",
  "ISNULL", "OFFSET", "RENAME", "SCHEMA", "SELECT", "SORTED", "TABLES",
  "UNIQUE", "UNLOAD", "UPDATE", "VALUES", "AFTER", "ALTER", "CROSS",
  "GROUP", "INDEX", "INNER", "LIMIT", "LOCAL", "MINUS", "ORDER", "OUTER",
  "RIGHT", "TABLE", "UNION", "USING", "WHERE", "CALL", "DESC", "DROP",
  "FILE", "FROM", "FULL", "HASH", "INTO", "JOIN", "LEFT", "LIKE", "LOAD",
  "NULL", "PLAN", "SHOW", "WITH", "ADD", "ALL", "AND", "ASC", "CSV", "FOR",
  "KEY", "NOT", "SET", "TBL", "TOP", "AS", "BY", "IF", "IN", "IS", "ON",
  "OR", "TO", "'='", "EQUALS", "'<'", "'>'", "LESS", "GREATER", "NOTNULL",
  "'+'", "'-'", "'*'", "'/'", "'%'", "'^'", "UMINUS", "'['", "']'", "'('",
  "')'", "'.'", "';'", "','", "$accept", "input", "statement_list",
  "statement", "import_statement", "import_file_type", "file_path",
  "create_statement", "opt_not_exists", "insert_statement",
  "opt_column_list", "select_statement", "select_with_paren",
  "select_no_paren", "set_operator", "select_clause", "select_list",
  "from_clause", "opt_where", "opt_group", "opt_order", "opt_order_type",
  "opt_limit", "expr_list", "literal_list", "expr_alias", "expr",
  "scalar_expr", "unary_expr", "binary_expr", "comp_expr", "function_expr",
  "column_name", "literal", "string_literal", "num_literal", "int_literal",
  "star_expr", "table_ref", "table_ref_atomic", "table_ref_commalist",
  "table_ref_name", "table_name", "alias", "opt_alias", "join_clause",
  "opt_join_type", "join_table", "join_condition", "opt_semicolon",
  "ident_commalist", YY_NULLPTR
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
     355,    61,   356,    60,    62,   357,   358,   359,    43,    45,
      42,    47,    37,    94,   360,    91,    93,    40,    41,    46,
      59,    44
};
# endif

#define YYPACT_NINF -150

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-150)))

#define YYTABLE_NINF -107

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-107)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -10,   -44,   -48,   -43,     1,   -28,    29,   -76,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,    10,   -33,   -35,    60,   -82,
    -150,  -150,  -150,     1,     1,  -150,     1,    -2,   -46,  -150,
      95,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,
    -150,  -150,   -42,   -40,  -150,   -10,  -150,  -150,  -150,    -8,
    -150,    41,    35,    20,    60,  -150,    30,   -11,    -5,     1,
     110,   209,    27,    16,     5,    49,     1,  -150,     1,     1,
       1,     1,     1,    40,   118,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,  -150,  -150,  -150,  -150,  -150,
       1,    72,   126,  -150,   113,    78,   145,   148,   149,   -17,
      57,  -150,  -150,   -28,  -150,    32,    19,     8,  -150,    75,
       1,   101,  -150,   223,   242,   242,   223,   209,     1,  -150,
     183,   223,   242,   242,    27,    27,    42,    42,    42,  -150,
     135,    35,   117,  -150,    68,    89,  -150,  -150,  -150,   -87,
      46,  -150,  -150,    58,     5,  -150,  -150,  -150,  -150,  -150,
     103,   165,    84,  -150,   223,  -150,  -150,  -150,  -150,   126,
     111,    60,  -150,   178,    66,     8,  -150,    61,     6,     1,
    -150,   145,  -150,  -150,   -79,  -150,    22,     5,   -28,  -150,
      85,   -46,  -150,  -150,    66,  -150,    67,     1,  -150,     8,
     165,  -150,  -150
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,   109,     3,     6,
       7,     8,     5,    19,    20,    36,    14,     0,     0,    76,
      80,    81,    83,     0,     0,    84,     0,     0,    29,    43,
      98,    49,    50,    51,    58,    52,    53,    55,    78,    79,
      82,    54,     0,     0,     1,   108,     2,    26,    27,     0,
      25,     0,    42,     0,     0,    10,     0,    93,    18,     0,
       0,    57,    56,     0,     0,    32,     0,    96,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    97,    47,    22,    21,     4,
       0,    36,     0,    23,     0,     0,     0,     0,     0,     0,
       0,    77,    48,     0,    30,    85,    87,    98,    89,   104,
       0,    34,    44,    70,    73,    74,    67,    65,     0,    95,
      66,    69,    71,    72,    60,    59,    62,    61,    63,    64,
      39,    42,    40,    13,     0,     0,    11,    94,   110,     0,
       0,    16,    75,     0,     0,    92,   100,   101,   103,   102,
       0,    31,     0,    28,    68,    38,    37,    35,    24,     0,
       0,     0,    17,     0,     0,     0,    90,    86,     0,     0,
      41,     0,     9,   111,     0,    45,    88,     0,     0,   106,
       0,    33,    12,    15,     0,    91,     0,     0,    46,     0,
     107,    99,   105
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -150,  -150,  -150,   141,  -150,  -150,    18,  -150,  -150,  -150,
    -150,   -93,   182,     7,  -150,   142,  -150,  -150,  -150,  -150,
     104,  -150,    69,    28,  -150,   143,   -23,  -150,  -150,  -150,
    -150,  -150,  -150,  -143,   -83,  -150,   -78,  -150,  -150,  -127,
    -150,    33,   -16,  -149,   105,  -150,  -150,    45,  -150,  -150,
    -150
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     6,     7,     8,     9,    56,   135,    10,    54,    11,
      99,    12,    13,    14,    51,    15,    27,    65,   111,   153,
      52,   157,    93,    28,   174,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,   104,   105,
     167,   106,   107,    85,    86,   108,   150,   109,   191,    46,
     139
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      61,    62,    58,    63,    19,    20,    21,    22,    57,    57,
     143,    67,    43,   136,   132,     4,   176,   166,    16,     1,
      47,   175,    17,    68,    69,    70,     4,     2,     3,    44,
      18,   162,   140,     4,   163,    59,   100,    60,    95,   183,
     192,   188,   184,    48,    45,   113,   114,   115,   116,   117,
     185,    55,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,    53,    57,    68,    69,    70,   130,    64,    49,
      20,    21,    22,    50,  -106,    66,    87,  -105,    88,  -106,
    -106,   170,  -105,  -105,     4,   186,    90,   151,   136,     5,
      23,    92,    71,  -106,  -106,   154,  -105,  -105,    67,    96,
      72,    74,    68,    69,    70,    73,   141,     5,    97,    94,
      24,    25,    98,   101,   110,    75,   118,    76,    26,    77,
      78,   119,   103,   178,    79,    80,    81,    82,    83,    84,
     146,    49,    22,    71,   102,   147,   148,    81,    82,    83,
      84,    72,    68,    69,    70,   172,    73,   133,   134,    20,
     149,   137,   138,   144,   152,    84,    75,   159,    76,   160,
      77,    78,   161,   164,   190,    79,    80,    81,    82,    83,
      84,    71,    68,    69,    70,   142,   165,   168,   169,    72,
     171,   173,   177,   187,    73,   189,    89,    42,    74,   182,
      68,    69,    70,    91,    75,   131,    76,   181,    77,    78,
     158,   179,   155,    79,    80,    81,    82,    83,    84,   112,
       0,    71,   145,   180,     0,     0,    68,    69,    70,    72,
     156,     0,     0,     0,    73,     0,     0,     0,     0,     0,
      68,    69,    70,     0,    75,     0,    76,     0,    77,    78,
       0,    71,     0,    79,    80,    81,    82,    83,    84,    72,
    -107,  -107,     0,     0,    73,     0,     0,     0,     0,    71,
       0,     0,     0,     0,    75,     0,    76,    72,    77,    78,
       0,     0,    73,    79,    80,    81,    82,    83,    84,     0,
       0,     0,     0,     0,    76,    71,    77,    78,     0,     0,
       0,    79,    80,    81,    82,    83,    84,     0,    73,    71,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      76,     0,    77,    78,     0,     0,     0,    79,    80,    81,
      82,    83,    84,     0,    76,     0,    77,    78,     0,     0,
       0,    79,    80,    81,    82,    83,    84,     0,     0,     0,
       0,     0,     0,     0,     0,  -107,  -107,     0,     0,     0,
      79,    80,    81,    82,    83,    84
};

static const yytype_int16 yycheck[] =
{
      23,    24,    18,    26,     3,     4,     5,     6,     3,     3,
     103,     3,     5,    96,    92,    43,   165,   144,    62,    29,
      10,   164,    70,     7,     8,     9,    43,    37,    38,     0,
      73,   118,    49,    43,   121,   117,    59,   119,    54,   118,
     189,   184,   121,    33,   120,    68,    69,    70,    71,    72,
     177,    86,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    95,     3,     7,     8,     9,    90,    70,    59,
       4,     5,     6,    63,    55,   121,   118,    55,   118,    60,
      61,   159,    60,    61,    43,   178,    94,   110,   171,   117,
      89,    56,    76,    74,    75,   118,    74,    75,     3,    69,
      84,    93,     7,     8,     9,    89,    99,   117,   119,    89,
     109,   110,   117,     3,    65,    99,    76,   101,   117,   103,
     104,     3,   117,   117,   108,   109,   110,   111,   112,   113,
      55,    59,     6,    76,   118,    60,    61,   110,   111,   112,
     113,    84,     7,     8,     9,   161,    89,    34,    70,     4,
      75,     3,     3,   121,    53,   113,    99,    40,   101,    91,
     103,   104,    73,   117,   187,   108,   109,   110,   111,   112,
     113,    76,     7,     8,     9,   118,   118,    74,    94,    84,
      69,     3,   121,    98,    89,   118,    45,     5,    93,   171,
       7,     8,     9,    51,    99,    91,   101,   169,   103,   104,
     131,   168,    67,   108,   109,   110,   111,   112,   113,    66,
      -1,    76,   107,   168,    -1,    -1,     7,     8,     9,    84,
      85,    -1,    -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,
       7,     8,     9,    -1,    99,    -1,   101,    -1,   103,   104,
      -1,    76,    -1,   108,   109,   110,   111,   112,   113,    84,
       8,     9,    -1,    -1,    89,    -1,    -1,    -1,    -1,    76,
      -1,    -1,    -1,    -1,    99,    -1,   101,    84,   103,   104,
      -1,    -1,    89,   108,   109,   110,   111,   112,   113,    -1,
      -1,    -1,    -1,    -1,   101,    76,   103,   104,    -1,    -1,
      -1,   108,   109,   110,   111,   112,   113,    -1,    89,    76,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     101,    -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,    -1,   101,    -1,   103,   104,    -1,    -1,
      -1,   108,   109,   110,   111,   112,   113,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,   104,    -1,    -1,    -1,
     108,   109,   110,   111,   112,   113
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    29,    37,    38,    43,   117,   123,   124,   125,   126,
     129,   131,   133,   134,   135,   137,    62,    70,    73,     3,
       4,     5,     6,    89,   109,   110,   117,   138,   145,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   134,   135,     0,   120,   171,    10,    33,    59,
      63,   136,   142,    95,   130,    86,   127,     3,   164,   117,
     119,   148,   148,   148,    70,   139,   121,     3,     7,     8,
       9,    76,    84,    89,    93,    99,   101,   103,   104,   108,
     109,   110,   111,   112,   113,   165,   166,   118,   118,   125,
      94,   137,    56,   144,    89,   164,    69,   119,   117,   132,
     148,     3,   118,   117,   160,   161,   163,   164,   167,   169,
      65,   140,   147,   148,   148,   148,   148,   148,    76,     3,
     148,   148,   148,   148,   148,   148,   148,   148,   148,   148,
     148,   142,   158,    34,    70,   128,   156,     3,     3,   172,
      49,   135,   118,   133,   121,   166,    55,    60,    61,    75,
     168,   148,    53,   141,   148,    67,    85,   143,   144,    40,
      91,    73,   118,   121,   117,   118,   161,   162,    74,    94,
     158,    69,   164,     3,   146,   155,   165,   121,   117,   163,
     169,   145,   128,   118,   121,   161,   133,    98,   155,   118,
     148,   170,   165
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   122,   123,   124,   124,   125,   125,   125,   125,   126,
     127,   128,   129,   130,   130,   131,   131,   132,   132,   133,
     133,   134,   134,   135,   135,   136,   136,   136,   137,   138,
     139,   140,   140,   141,   141,   142,   142,   143,   143,   143,
     144,   144,   144,   145,   145,   146,   146,   147,   148,   148,
     148,   148,   148,   149,   149,   149,   150,   150,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   152,
     152,   152,   152,   152,   152,   153,   154,   154,   155,   155,
     156,   157,   157,   158,   159,   160,   160,   161,   161,   161,
     162,   162,   163,   164,   164,   165,   165,   166,   166,   167,
     168,   168,   168,   168,   168,   169,   169,   170,   171,   171,
     172,   172
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     1,     3,     1,     1,     1,     1,     7,
       1,     1,     8,     3,     0,     8,     5,     3,     0,     1,
       1,     3,     3,     3,     5,     1,     1,     1,     5,     1,
       2,     2,     0,     3,     0,     4,     0,     1,     1,     0,
       2,     4,     0,     1,     3,     1,     3,     2,     3,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     4,     3,
       3,     3,     3,     3,     3,     4,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     4,     1,
       1,     3,     2,     1,     3,     2,     1,     1,     0,     6,
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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, hsql::StatementList** result, yyscan_t scanner)
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, hsql::StatementList** result, yyscan_t scanner)
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, hsql::StatementList** result, yyscan_t scanner)
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, hsql::StatementList** result, yyscan_t scanner)
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
yyparse (hsql::StatementList** result, yyscan_t scanner)
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
#line 177 "bison_parser.y" /* yacc.c:1646  */
    { *result = (yyvsp[-1].stmt_list); }
#line 1604 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 182 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = new StatementList((yyvsp[0].statement)); }
#line 1610 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 183 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].stmt_list)->push_back((yyvsp[0].statement)); (yyval.stmt_list) = (yyvsp[-2].stmt_list); }
#line 1616 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 187 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].select_stmt); }
#line 1622 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 188 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].import_stmt); }
#line 1628 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 189 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].create_stmt); }
#line 1634 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 190 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].insert_stmt); }
#line 1640 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 199 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.import_stmt) = new ImportStatement();
			(yyval.import_stmt)->file_type = (ImportFileType) (yyvsp[-4].uval);
			(yyval.import_stmt)->file_path = (yyvsp[-2].sval);
			(yyval.import_stmt)->table_name = (yyvsp[0].sval);
		}
#line 1651 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 208 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = kImportCSV; }
#line 1657 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 212 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.sval) = (yyvsp[0].expr)->name; }
#line 1663 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 220 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.create_stmt) = new CreateStatement();
			(yyval.create_stmt)->create_type = kTableFromTbl;
			(yyval.create_stmt)->if_not_exists = (yyvsp[-5].bval);
			(yyval.create_stmt)->table_name = (yyvsp[-4].sval);
			(yyval.create_stmt)->file_path = (yyvsp[0].sval);
		}
#line 1675 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 230 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.bval) = true; }
#line 1681 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 231 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.bval) = false; }
#line 1687 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 239 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.insert_stmt) = new InsertStatement();
			(yyval.insert_stmt)->insert_type = InsertStatement::kInsertValues;
			(yyval.insert_stmt)->table_name = (yyvsp[-5].sval);
			(yyval.insert_stmt)->columns = (yyvsp[-4].slist);
			(yyval.insert_stmt)->values = (yyvsp[-1].expr_list);
		}
#line 1699 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 246 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.insert_stmt) = new InsertStatement();
			(yyval.insert_stmt)->insert_type = InsertStatement::kInsertSelect;
			(yyval.insert_stmt)->table_name = (yyvsp[-2].sval);
			(yyval.insert_stmt)->columns = (yyvsp[-1].slist);
			(yyval.insert_stmt)->select = (yyvsp[0].select_stmt);
		}
#line 1711 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 257 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.slist) = (yyvsp[-1].slist); }
#line 1717 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 258 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.slist) = NULL; }
#line 1723 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 271 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.select_stmt) = (yyvsp[-1].select_stmt); }
#line 1729 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 272 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.select_stmt) = (yyvsp[-1].select_stmt); }
#line 1735 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 276 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.select_stmt) = (yyvsp[-2].select_stmt);
			(yyval.select_stmt)->order = (yyvsp[-1].order);
			(yyval.select_stmt)->limit = (yyvsp[0].limit);
		}
#line 1745 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 281 "bison_parser.y" /* yacc.c:1646  */
    {
			// TODO: allow multiple unions (through linked list)
			// TODO: capture type of set_operator
			// TODO: might overwrite order and limit of first select here
			(yyval.select_stmt) = (yyvsp[-4].select_stmt);
			(yyval.select_stmt)->union_select = (yyvsp[-2].select_stmt);
			(yyval.select_stmt)->order = (yyvsp[-1].order);
			(yyval.select_stmt)->limit = (yyvsp[0].limit);
		}
#line 1759 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 299 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.select_stmt) = new SelectStatement();
			(yyval.select_stmt)->select_list = (yyvsp[-3].expr_list);
			(yyval.select_stmt)->from_table = (yyvsp[-2].table);
			(yyval.select_stmt)->where_clause = (yyvsp[-1].expr);
			(yyval.select_stmt)->group_by = (yyvsp[0].expr_list);
		}
#line 1771 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 315 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.table) = (yyvsp[0].table); }
#line 1777 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 320 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 1783 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 321 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = NULL; }
#line 1789 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 326 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = (yyvsp[0].expr_list); }
#line 1795 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 327 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = NULL; }
#line 1801 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 332 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.order) = new OrderDescription((yyvsp[0].order_type), (yyvsp[-1].expr)); }
#line 1807 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 333 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.order) = NULL; }
#line 1813 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 337 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.order_type) = kOrderAsc; }
#line 1819 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 338 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.order_type) = kOrderDesc; }
#line 1825 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 339 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.order_type) = kOrderAsc; }
#line 1831 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 344 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.limit) = new LimitDescription((yyvsp[0].expr)->ival, kNoOffset); delete (yyvsp[0].expr); }
#line 1837 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 345 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.limit) = new LimitDescription((yyvsp[-2].expr)->ival, (yyvsp[0].expr)->ival); delete (yyvsp[-2].expr); delete (yyvsp[0].expr); }
#line 1843 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 346 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.limit) = NULL; }
#line 1849 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 43:
#line 353 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = new List<Expr*>((yyvsp[0].expr)); }
#line 1855 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 44:
#line 354 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].expr_list)->push_back((yyvsp[0].expr)); (yyval.expr_list) = (yyvsp[-2].expr_list); }
#line 1861 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 358 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = new List<Expr*>((yyvsp[0].expr)); }
#line 1867 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 46:
#line 359 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].expr_list)->push_back((yyvsp[0].expr)); (yyval.expr_list) = (yyvsp[-2].expr_list); }
#line 1873 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 363 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.expr) = (yyvsp[-1].expr);
			(yyval.expr)->alias = (yyvsp[0].sval);
		}
#line 1882 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 370 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); }
#line 1888 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 384 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpUnary(Expr::UMINUS, (yyvsp[0].expr)); }
#line 1894 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 385 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpUnary(Expr::NOT, (yyvsp[0].expr)); }
#line 1900 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 390 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '-', (yyvsp[0].expr)); }
#line 1906 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 391 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '+', (yyvsp[0].expr)); }
#line 1912 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 392 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '/', (yyvsp[0].expr)); }
#line 1918 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 393 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '*', (yyvsp[0].expr)); }
#line 1924 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 394 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '%', (yyvsp[0].expr)); }
#line 1930 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 395 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '^', (yyvsp[0].expr)); }
#line 1936 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 65:
#line 396 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::AND, (yyvsp[0].expr)); }
#line 1942 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 397 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::OR, (yyvsp[0].expr)); }
#line 1948 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 398 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::LIKE, (yyvsp[0].expr)); }
#line 1954 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 68:
#line 399 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-3].expr), Expr::NOT_LIKE, (yyvsp[0].expr)); }
#line 1960 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 69:
#line 404 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '=', (yyvsp[0].expr)); }
#line 1966 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 70:
#line 405 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::NOT_EQUALS, (yyvsp[0].expr)); }
#line 1972 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 71:
#line 406 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '<', (yyvsp[0].expr)); }
#line 1978 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 72:
#line 407 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '>', (yyvsp[0].expr)); }
#line 1984 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 73:
#line 408 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::LESS_EQ, (yyvsp[0].expr)); }
#line 1990 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 74:
#line 409 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::GREATER_EQ, (yyvsp[0].expr)); }
#line 1996 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 413 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeFunctionRef((yyvsp[-3].sval), (yyvsp[-1].expr)); }
#line 2002 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 417 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeColumnRef((yyvsp[0].sval)); }
#line 2008 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 77:
#line 418 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeColumnRef((yyvsp[-2].sval), (yyvsp[0].sval)); }
#line 2014 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 80:
#line 427 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeLiteral((yyvsp[0].sval)); }
#line 2020 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 81:
#line 432 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeLiteral((yyvsp[0].fval)); }
#line 2026 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 83:
#line 437 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeLiteral((yyvsp[0].ival)); }
#line 2032 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 441 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = new Expr(kExprStar); }
#line 2038 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 86:
#line 450 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyvsp[0].table_list)->push_back((yyvsp[-2].table));
			auto tbl = new TableRef(kTableCrossProduct);
			tbl->list = (yyvsp[0].table_list);
			(yyval.table) = tbl;
		}
#line 2049 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 461 "bison_parser.y" /* yacc.c:1646  */
    {
			auto tbl = new TableRef(kTableSelect);
			tbl->select = (yyvsp[-2].select_stmt);
			tbl->alias = (yyvsp[0].sval);
			(yyval.table) = tbl;
		}
#line 2060 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 472 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.table_list) = new List<TableRef*>((yyvsp[0].table)); }
#line 2066 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 91:
#line 473 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].table_list)->push_back((yyvsp[0].table)); (yyval.table_list) = (yyvsp[-2].table_list); }
#line 2072 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 478 "bison_parser.y" /* yacc.c:1646  */
    {
			auto tbl = new TableRef(kTableName);
			tbl->name = (yyvsp[-1].sval);
			tbl->alias = (yyvsp[0].sval);
			(yyval.table) = tbl;
		}
#line 2083 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 95:
#line 493 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.sval) = (yyvsp[0].sval); }
#line 2089 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 98:
#line 499 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.sval) = NULL; }
#line 2095 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 99:
#line 508 "bison_parser.y" /* yacc.c:1646  */
    { 
			(yyval.table) = new TableRef(kTableJoin);
			(yyval.table)->join = new JoinDefinition();
			(yyval.table)->join->type = (JoinType) (yyvsp[-4].uval);
			(yyval.table)->join->left = (yyvsp[-5].table);
			(yyval.table)->join->right = (yyvsp[-2].table);
			(yyval.table)->join->condition = (yyvsp[0].expr);
		}
#line 2108 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 519 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = kJoinInner; }
#line 2114 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 101:
#line 520 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = kJoinOuter; }
#line 2120 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 521 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = kJoinLeft; }
#line 2126 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 522 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = kJoinRight; }
#line 2132 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 523 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = kJoinInner; }
#line 2138 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 529 "bison_parser.y" /* yacc.c:1646  */
    {
			auto tbl = new TableRef(kTableSelect);
			tbl->select = (yyvsp[-2].select_stmt);
			tbl->alias = (yyvsp[0].sval);
			(yyval.table) = tbl;
		}
#line 2149 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 554 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.slist) = new List<char*>((yyvsp[0].sval)); }
#line 2155 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 555 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].slist)->push_back((yyvsp[0].sval)); (yyval.slist) = (yyvsp[-2].slist); }
#line 2161 "bison_parser.cpp" /* yacc.c:1646  */
    break;


#line 2165 "bison_parser.cpp" /* yacc.c:1646  */
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
#line 558 "bison_parser.y" /* yacc.c:1906  */

/*********************************
 ** Section 4: Additional C code
 *********************************/

/* empty */

