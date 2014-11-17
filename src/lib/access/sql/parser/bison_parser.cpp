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

	hsql::TableRef* table;
	hsql::Expr* expr;
	hsql::OrderDescription* order;
	hsql::OrderType order_type;
	hsql::LimitDescription* limit;

	hsql::StatementList* stmt_list;
	hsql::List<char*>* slist;
	hsql::List<hsql::Expr*>* expr_list;
	hsql::List<hsql::TableRef*>* table_list;

#line 261 "bison_parser.cpp" /* yacc.c:355  */
};
# define HSQL_STYPE_IS_TRIVIAL 1
# define HSQL_STYPE_IS_DECLARED 1
#endif



int hsql_parse (hsql::StatementList** result, yyscan_t scanner);

#endif /* !YY_HSQL_BISON_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 275 "bison_parser.cpp" /* yacc.c:358  */

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
#define YYFINAL  42
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   360

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  86
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  46
/* YYNRULES -- Number of rules.  */
#define YYNRULES  96
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  169

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   324

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    76,     2,     2,
      81,    82,    74,    72,    85,    73,    83,    75,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    84,
      67,    65,    68,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    79,     2,    80,    77,     2,     2,     2,     2,     2,
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
      66,    69,    70,    71,    78
};

#if HSQL_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   172,   172,   177,   178,   185,   186,   187,   196,   205,
     209,   217,   227,   228,   236,   237,   241,   242,   246,   251,
     261,   262,   266,   277,   282,   287,   288,   293,   294,   299,
     300,   304,   305,   306,   311,   312,   313,   320,   321,   325,
     332,   333,   334,   335,   336,   340,   341,   342,   346,   347,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   366,   367,   368,   369,   370,   371,   375,   379,   380,
     384,   385,   389,   394,   395,   399,   403,   411,   412,   422,
     423,   429,   434,   435,   440,   449,   450,   455,   456,   460,
     461,   469,   481,   487,   491,   500,   501
};
#endif

#if HSQL_DEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENTIFIER", "STRING", "FLOAT", "INT",
  "NOTEQUALS", "LESSEQ", "GREATEREQ", "DATABASE", "DISTINCT", "BETWEEN",
  "CONTROL", "NATURAL", "COLUMN", "CREATE", "DELETE", "EXISTS", "HAVING",
  "IMPORT", "INSERT", "ISNULL", "OFFSET", "RENAME", "SELECT", "UNLOAD",
  "UPDATE", "ALTER", "CROSS", "GROUP", "INDEX", "INNER", "LIMIT", "ORDER",
  "OUTER", "RIGHT", "TABLE", "UNION", "USING", "WHERE", "DESC", "DROP",
  "FILE", "FROM", "INTO", "JOIN", "LEFT", "LIKE", "LOAD", "NULL", "ALL",
  "AND", "ASC", "CSV", "NOT", "TBL", "TOP", "AS", "BY", "IF", "IN", "IS",
  "ON", "OR", "'='", "EQUALS", "'<'", "'>'", "LESS", "GREATER", "NOTNULL",
  "'+'", "'-'", "'*'", "'/'", "'%'", "'^'", "UMINUS", "'['", "']'", "'('",
  "')'", "'.'", "';'", "','", "$accept", "input", "statement_list",
  "statement", "import_statement", "import_file_type", "file_path",
  "create_statement", "opt_not_exists", "select_statement",
  "select_with_paren", "select_no_paren", "select_ref", "select_clause",
  "select_list", "from_clause", "opt_where", "opt_group", "opt_order",
  "opt_order_type", "opt_limit", "expr_list", "expr_alias", "expr",
  "scalar_expr", "unary_expr", "binary_expr", "comp_expr", "function_expr",
  "column_name", "literal", "string_literal", "num_literal", "int_literal",
  "star_expr", "table_ref", "table_ref_atomic", "table_ref_commalist",
  "table_ref_name", "table_name", "alias", "opt_alias", "join_clause",
  "join_table", "join_condition", "opt_semicolon", YY_NULLPTR
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
     315,   316,   317,   318,   319,    61,   320,    60,    62,   321,
     322,   323,    43,    45,    42,    47,    37,    94,   324,    91,
      93,    40,    41,    46,    59,    44
};
# endif

#define YYPACT_NINF -137

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-137)))

#define YYTABLE_NINF -94

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-94)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      -3,    -7,   -12,     2,   -11,    33,   -50,  -137,  -137,  -137,
    -137,     1,  -137,    20,   -14,   -25,    -8,   -60,  -137,  -137,
    -137,     2,     2,  -137,     2,    16,   -23,  -137,   110,  -137,
    -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,
     -19,   -18,  -137,    -3,  -137,   -11,    13,    40,    19,    77,
    -137,    42,     2,    83,   235,   -49,    29,     6,    49,     2,
    -137,     2,     2,     2,     2,     2,    43,    89,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,  -137,  -137,
    -137,  -137,  -137,  -137,    61,  -137,     2,    92,  -137,    81,
      24,    65,   106,    60,  -137,  -137,   -11,  -137,    31,    68,
       8,  -137,    74,     2,    91,  -137,   265,   283,   283,   265,
     235,     2,  -137,   212,   265,   283,   283,   -49,   -49,    45,
      45,    45,  -137,    40,   140,   100,  -137,   123,    73,    85,
    -137,  -137,    56,     6,  -137,     7,   182,    72,  -137,   265,
    -137,  -137,  -137,  -137,    92,  -137,    96,    77,     8,  -137,
      58,   -11,  -137,    78,     2,  -137,   106,  -137,    98,     6,
      63,     2,   -23,  -137,  -137,     8,   182,  -137,  -137
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,    96,     3,     6,     7,
       5,    14,    15,     0,    30,    13,     0,    68,    72,    73,
      75,     0,     0,    76,     0,     0,    23,    37,    90,    41,
      42,    43,    50,    44,    45,    47,    70,    71,    74,    46,
      21,     0,     1,    95,     2,     0,     0,    36,     0,     0,
       9,     0,     0,     0,    49,    48,     0,     0,    26,     0,
      88,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    89,    39,
      17,    16,     4,    21,    30,    20,     0,     0,    18,     0,
      85,     0,     0,     0,    69,    40,     0,    24,    77,    79,
      90,    81,     0,     0,    28,    38,    62,    65,    66,    59,
      57,     0,    87,    58,    61,    63,    64,    52,    51,    54,
      53,    55,    56,    36,    33,    34,    12,     0,     0,     0,
      10,    67,     0,     0,    84,     0,    25,     0,    22,    60,
      19,    32,    31,    29,     0,    86,     0,     0,     0,    82,
      78,     0,    93,     0,     0,    35,     0,     8,    80,     0,
       0,     0,    27,    11,    83,     0,    94,    91,    92
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -137,  -137,  -137,   103,  -137,  -137,    -6,  -137,  -137,   -80,
      14,   147,   107,   108,  -137,  -137,  -137,  -137,    70,  -137,
      32,     3,    97,   -21,  -137,  -137,  -137,  -137,  -137,  -137,
    -137,   -77,  -137,   -83,  -137,  -137,  -114,  -137,    25,   -47,
    -136,    59,  -137,    26,  -137,  -137
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     5,     6,     7,     8,    51,   129,     9,    49,    10,
      11,    12,    13,    14,    25,    58,   104,   138,    47,   143,
      88,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    97,    98,   150,    99,   100,
      78,    79,   101,   102,   167,    44
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      54,    55,    91,    56,   125,    17,    18,    19,    20,    90,
      90,    60,   158,     1,     3,   130,   132,     2,    40,   149,
      46,    52,     3,    53,   -20,    74,    75,    76,    77,   168,
      15,    93,    16,    42,    43,    48,    61,    62,    63,   -21,
     106,   107,   108,   109,   110,   164,    50,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,    21,    45,    83,
      57,   155,    59,    80,    81,   124,    67,    61,    62,    63,
       4,   160,    86,    87,    89,    22,    23,    64,     4,   130,
      90,    65,   136,    24,    66,    92,    94,    96,   151,   103,
     139,   111,   112,    68,    69,    46,    70,    71,    20,   126,
     157,    72,    73,    74,    75,    76,    77,   127,    64,   128,
      18,    95,    65,    60,   -93,    66,   133,    61,    62,    63,
     135,   137,    77,   144,    68,    69,   145,    70,    71,   146,
     147,   154,    72,    73,    74,    75,    76,    77,   148,   156,
     166,   161,   131,   159,   -92,   165,    82,    61,    62,    63,
     163,    41,    84,    85,   123,   140,   105,   162,    64,   134,
     152,   153,    65,     0,     0,    66,     0,     0,    67,     0,
       0,     0,     0,     0,    68,    69,     0,    70,    71,     0,
       0,   141,    72,    73,    74,    75,    76,    77,    64,    61,
      62,    63,    65,   142,     0,    66,     0,     0,     0,     0,
       0,     0,     0,     0,    68,    69,     0,    70,    71,     0,
       0,     0,    72,    73,    74,    75,    76,    77,     0,    61,
      62,    63,     0,     0,     0,     0,     0,     0,     0,     0,
      64,     0,     0,     0,    65,     0,     0,    66,     0,     0,
       0,     0,    61,    62,    63,     0,    68,    69,     0,    70,
      71,     0,     0,     0,    72,    73,    74,    75,    76,    77,
      64,     0,     0,     0,    65,     0,     0,    66,     0,     0,
       0,     0,    61,    62,    63,     0,     0,    69,     0,    70,
      71,     0,     0,    64,    72,    73,    74,    75,    76,    77,
      66,   -94,   -94,     0,     0,     0,     0,     0,     0,     0,
      69,     0,    70,    71,     0,     0,     0,    72,    73,    74,
      75,    76,    77,    64,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      69,     0,    70,    71,     0,     0,     0,    72,    73,    74,
      75,    76,    77,     0,     0,     0,     0,     0,     0,     0,
     -94,   -94,     0,     0,     0,    72,    73,    74,    75,    76,
      77
};

static const yytype_int16 yycheck[] =
{
      21,    22,    49,    24,    87,     3,     4,     5,     6,     3,
       3,     3,   148,    16,    25,    92,    96,    20,     4,   133,
      34,    81,    25,    83,    38,    74,    75,    76,    77,   165,
      37,    52,    44,     0,    84,    60,     7,     8,     9,    38,
      61,    62,    63,    64,    65,   159,    54,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    55,    38,    45,
      44,   144,    85,    82,    82,    86,    58,     7,     8,     9,
      81,   151,    59,    33,    55,    73,    74,    48,    81,   156,
       3,    52,   103,    81,    55,    43,     3,    81,    81,    40,
     111,    48,     3,    64,    65,    34,    67,    68,     6,    18,
     147,    72,    73,    74,    75,    76,    77,    83,    48,    44,
       4,    82,    52,     3,    46,    55,    85,     7,     8,     9,
      46,    30,    77,    23,    64,    65,     3,    67,    68,    56,
      45,    59,    72,    73,    74,    75,    76,    77,    82,    43,
     161,    63,    82,    85,    46,    82,    43,     7,     8,     9,
     156,     4,    45,    45,    84,   123,    59,   154,    48,   100,
     135,   135,    52,    -1,    -1,    55,    -1,    -1,    58,    -1,
      -1,    -1,    -1,    -1,    64,    65,    -1,    67,    68,    -1,
      -1,    41,    72,    73,    74,    75,    76,    77,    48,     7,
       8,     9,    52,    53,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    64,    65,    -1,    67,    68,    -1,
      -1,    -1,    72,    73,    74,    75,    76,    77,    -1,     7,
       8,     9,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    -1,    -1,    52,    -1,    -1,    55,    -1,    -1,
      -1,    -1,     7,     8,     9,    -1,    64,    65,    -1,    67,
      68,    -1,    -1,    -1,    72,    73,    74,    75,    76,    77,
      48,    -1,    -1,    -1,    52,    -1,    -1,    55,    -1,    -1,
      -1,    -1,     7,     8,     9,    -1,    -1,    65,    -1,    67,
      68,    -1,    -1,    48,    72,    73,    74,    75,    76,    77,
      55,     8,     9,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    67,    68,    -1,    -1,    -1,    72,    73,    74,
      75,    76,    77,    48,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    67,    68,    -1,    -1,    -1,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    68,    -1,    -1,    -1,    72,    73,    74,    75,    76,
      77
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    16,    20,    25,    81,    87,    88,    89,    90,    93,
      95,    96,    97,    98,    99,    37,    44,     3,     4,     5,
       6,    55,    73,    74,    81,   100,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
      96,    97,     0,    84,   131,    38,    34,   104,    60,    94,
      54,    91,    81,    83,   109,   109,   109,    44,   101,    85,
       3,     7,     8,     9,    48,    52,    55,    58,    64,    65,
      67,    68,    72,    73,    74,    75,    76,    77,   126,   127,
      82,    82,    89,    96,    98,    99,    59,    33,   106,    55,
       3,   125,    43,   109,     3,    82,    81,   121,   122,   124,
     125,   128,   129,    40,   102,   108,   109,   109,   109,   109,
     109,    48,     3,   109,   109,   109,   109,   109,   109,   109,
     109,   109,   109,   104,   109,   119,    18,    83,    44,    92,
     117,    82,    95,    85,   127,    46,   109,    30,   103,   109,
     106,    41,    53,   105,    23,     3,    56,    45,    82,   122,
     123,    81,   124,   129,    59,   119,    43,   125,   126,    85,
      95,    63,   107,    92,   122,    82,   109,   130,   126
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    86,    87,    88,    88,    89,    89,    89,    90,    91,
      92,    93,    94,    94,    95,    95,    96,    96,    97,    97,
      98,    98,    99,   100,   101,   102,   102,   103,   103,   104,
     104,   105,   105,   105,   106,   106,   106,   107,   107,   108,
     109,   109,   109,   109,   109,   110,   110,   110,   111,   111,
     112,   112,   112,   112,   112,   112,   112,   112,   112,   112,
     112,   113,   113,   113,   113,   113,   113,   114,   115,   115,
     116,   116,   117,   118,   118,   119,   120,   121,   121,   122,
     122,   122,   123,   123,   124,   125,   125,   126,   126,   127,
     127,   128,   129,   129,   130,   131,   131
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     1,     3,     1,     1,     1,     7,     1,
       1,     8,     3,     0,     1,     1,     3,     3,     3,     5,
       1,     1,     5,     1,     2,     2,     0,     3,     0,     4,
       0,     1,     1,     0,     2,     4,     0,     1,     3,     2,
       3,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       4,     3,     3,     3,     3,     3,     3,     4,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       4,     1,     1,     3,     2,     1,     3,     2,     1,     1,
       0,     5,     4,     1,     1,     1,     0
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
#line 172 "bison_parser.y" /* yacc.c:1646  */
    { *result = (yyvsp[-1].stmt_list); }
#line 1537 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 177 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.stmt_list) = new StatementList((yyvsp[0].statement)); }
#line 1543 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 178 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].stmt_list)->push_back((yyvsp[0].statement)); (yyval.stmt_list) = (yyvsp[-2].stmt_list); }
#line 1549 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 185 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].select_stmt); }
#line 1555 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 186 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].import_stmt); }
#line 1561 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 187 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].create_stmt); }
#line 1567 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 196 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.import_stmt) = new ImportStatement();
			(yyval.import_stmt)->file_type = (ImportFileType) (yyvsp[-4].uval);
			(yyval.import_stmt)->file_path = (yyvsp[-2].sval);
			(yyval.import_stmt)->table_name = (yyvsp[0].sval);
		}
#line 1578 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 205 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.uval) = kImportCSV; }
#line 1584 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 209 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.sval) = (yyvsp[0].expr)->name; }
#line 1590 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 217 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.create_stmt) = new CreateStatement();
			(yyval.create_stmt)->create_type = kTableFromTbl;
			(yyval.create_stmt)->if_not_exists = (yyvsp[-5].bval);
			(yyval.create_stmt)->table_name = (yyvsp[-4].sval);
			(yyval.create_stmt)->file_path = (yyvsp[0].sval);
		}
#line 1602 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 227 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.bval) = true; }
#line 1608 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 228 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.bval) = false; }
#line 1614 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 241 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.select_stmt) = (yyvsp[-1].select_stmt); }
#line 1620 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 242 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.select_stmt) = (yyvsp[-1].select_stmt); }
#line 1626 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 246 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.select_stmt) = (yyvsp[-2].select_stmt);
			(yyval.select_stmt)->order = (yyvsp[-1].order);
			(yyval.select_stmt)->limit = (yyvsp[0].limit);
		}
#line 1636 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 251 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.select_stmt) = (yyvsp[-4].select_stmt);
			(yyval.select_stmt)->union_select = (yyvsp[-2].select_stmt);
			// TODO: might overwrite order and limit of first select here
			(yyval.select_stmt)->order = (yyvsp[-1].order);
			(yyval.select_stmt)->limit = (yyvsp[0].limit);
		}
#line 1648 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 266 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.select_stmt) = new SelectStatement();
			(yyval.select_stmt)->select_list = (yyvsp[-3].expr_list);
			(yyval.select_stmt)->from_table = (yyvsp[-2].table);
			(yyval.select_stmt)->where_clause = (yyvsp[-1].expr);
			(yyval.select_stmt)->group_by = (yyvsp[0].expr_list);
		}
#line 1660 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 282 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.table) = (yyvsp[0].table); }
#line 1666 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 287 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 1672 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 288 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = NULL; }
#line 1678 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 293 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = (yyvsp[0].expr_list); }
#line 1684 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 294 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = NULL; }
#line 1690 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 299 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.order) = new OrderDescription((yyvsp[0].order_type), (yyvsp[-1].expr)); }
#line 1696 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 300 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.order) = NULL; }
#line 1702 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 304 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.order_type) = kOrderAsc; }
#line 1708 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 305 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.order_type) = kOrderDesc; }
#line 1714 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 306 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.order_type) = kOrderAsc; }
#line 1720 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 311 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.limit) = new LimitDescription((yyvsp[0].expr)->ival, kNoOffset); delete (yyvsp[0].expr); }
#line 1726 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 312 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.limit) = new LimitDescription((yyvsp[-2].expr)->ival, (yyvsp[0].expr)->ival); delete (yyvsp[-2].expr); delete (yyvsp[0].expr); }
#line 1732 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 313 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.limit) = NULL; }
#line 1738 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 320 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr_list) = new List<Expr*>((yyvsp[0].expr)); }
#line 1744 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 321 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].expr_list)->push_back((yyvsp[0].expr)); (yyval.expr_list) = (yyvsp[-2].expr_list); }
#line 1750 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 325 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyval.expr) = (yyvsp[-1].expr);
			(yyval.expr)->alias = (yyvsp[0].sval);
		}
#line 1759 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 332 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); }
#line 1765 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 346 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpUnary(Expr::UMINUS, (yyvsp[0].expr)); }
#line 1771 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 49:
#line 347 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpUnary(Expr::NOT, (yyvsp[0].expr)); }
#line 1777 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 51:
#line 352 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '-', (yyvsp[0].expr)); }
#line 1783 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 52:
#line 353 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '+', (yyvsp[0].expr)); }
#line 1789 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 53:
#line 354 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '/', (yyvsp[0].expr)); }
#line 1795 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 54:
#line 355 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '*', (yyvsp[0].expr)); }
#line 1801 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 55:
#line 356 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '%', (yyvsp[0].expr)); }
#line 1807 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 357 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '^', (yyvsp[0].expr)); }
#line 1813 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 358 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::AND, (yyvsp[0].expr)); }
#line 1819 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 359 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::OR, (yyvsp[0].expr)); }
#line 1825 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 360 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::LIKE, (yyvsp[0].expr)); }
#line 1831 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 361 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-3].expr), Expr::NOT_LIKE, (yyvsp[0].expr)); }
#line 1837 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 366 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '=', (yyvsp[0].expr)); }
#line 1843 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 367 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::NOT_EQUALS, (yyvsp[0].expr)); }
#line 1849 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 368 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '<', (yyvsp[0].expr)); }
#line 1855 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 369 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), '>', (yyvsp[0].expr)); }
#line 1861 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 65:
#line 370 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::LESS_EQ, (yyvsp[0].expr)); }
#line 1867 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 371 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeOpBinary((yyvsp[-2].expr), Expr::GREATER_EQ, (yyvsp[0].expr)); }
#line 1873 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 375 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeFunctionRef((yyvsp[-3].sval), (yyvsp[-1].expr)); }
#line 1879 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 68:
#line 379 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeColumnRef((yyvsp[0].sval)); }
#line 1885 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 69:
#line 380 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeColumnRef((yyvsp[-2].sval), (yyvsp[0].sval)); }
#line 1891 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 72:
#line 389 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeLiteral((yyvsp[0].sval)); }
#line 1897 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 73:
#line 394 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeLiteral((yyvsp[0].fval)); }
#line 1903 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 399 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = Expr::makeLiteral((yyvsp[0].ival)); }
#line 1909 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 403 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.expr) = new Expr(kExprStar); }
#line 1915 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 78:
#line 412 "bison_parser.y" /* yacc.c:1646  */
    {
			(yyvsp[0].table_list)->push_back((yyvsp[-2].table));
			auto tbl = new TableRef(kTableCrossProduct);
			tbl->list = (yyvsp[0].table_list);
			(yyval.table) = tbl;
		}
#line 1926 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 80:
#line 423 "bison_parser.y" /* yacc.c:1646  */
    {
			auto tbl = new TableRef(kTableSelect);
			tbl->select = (yyvsp[-2].select_stmt);
			tbl->alias = (yyvsp[0].sval);
			(yyval.table) = tbl;
		}
#line 1937 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 82:
#line 434 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.table_list) = new List<TableRef*>((yyvsp[0].table)); }
#line 1943 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 83:
#line 435 "bison_parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].table_list)->push_back((yyvsp[0].table)); (yyval.table_list) = (yyvsp[-2].table_list); }
#line 1949 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 440 "bison_parser.y" /* yacc.c:1646  */
    {
			auto tbl = new TableRef(kTableName);
			tbl->name = (yyvsp[-1].sval);
			tbl->alias = (yyvsp[0].sval);
			(yyval.table) = tbl;
		}
#line 1960 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 87:
#line 455 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.sval) = (yyvsp[0].sval); }
#line 1966 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 461 "bison_parser.y" /* yacc.c:1646  */
    { (yyval.sval) = NULL; }
#line 1972 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 91:
#line 470 "bison_parser.y" /* yacc.c:1646  */
    { 
			(yyval.table) = new TableRef(kTableJoin);
			(yyval.table)->left = (yyvsp[-4].table);
			(yyval.table)->right = (yyvsp[-2].table);
			(yyval.table)->join_condition = (yyvsp[0].expr);
			(yyval.table)->join_type = kJoinInner;
		}
#line 1984 "bison_parser.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 481 "bison_parser.y" /* yacc.c:1646  */
    {
			auto tbl = new TableRef(kTableSelect);
			tbl->select = (yyvsp[-2].select_stmt);
			tbl->alias = (yyvsp[0].sval);
			(yyval.table) = tbl;
		}
#line 1995 "bison_parser.cpp" /* yacc.c:1646  */
    break;


#line 1999 "bison_parser.cpp" /* yacc.c:1646  */
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
#line 505 "bison_parser.y" /* yacc.c:1906  */

/*********************************
 ** Section 4: Additional C code
 *********************************/

/* empty */

