#Just Another expressioN system

This expression system enables you to generate expressions before compile time.

## Architecture
The system consists of several components:
### expressionsToGenerate text file
This text file simply includes strings defining the expressions which should be generated. It also includes a fixed set of expressions, these are necessary to make the tests run properly. However, the user is able to call these expressions as if they would be manually defined.
### ExpressionTemplate files
These files are the skeleton for the generated expressions and contain template instructions to build later the actual implementations. The template engine is *Jinja2*.
### generator.py
This python script reads the content of the expressionsToGenerate file and processes it line by line. After processing, it applies the gathered information to the template files and generates a header and implementation file for each expression. The results are stored in a folder called *generatedExpressions*.

## Expression syntax
An expression can consist of several sub-expressions. At least one sub-expression is necessary to generate a working expression. A sub-expression has to contain an operator and a datatype, for example *EQ_INT*. The order is fixed. Sub-expressions must be connected by using either *AND* or *OR* in the current implementation state. Every word is delimited by underscores.

- \<Expression> ::= \<Operator>\_\<Type> | \<Expression>\_\<Connection>\_\<Expression>
- \<Expression> ::= \<BraceOpen>\_\<Expression> | \<Expression>_\<BraceClose>
- \<Operator> ::= EQ | GT | LT | GTEQ | LTEQ
- \<Type> ::= INT | FLOAT | STRING
- \<Connection> ::= AND | OR
- \<BraceOpen> ::= (
- \<BraceClose> ::= )


EQ_INT_OR_GT_FLOAT generates an evaluation command like:

	int == searchValue1 || float > searchValue2

LTEQ_INT_OR\_(\_GT_FLOAT_AND_EQ_STRING_) generates an evaluation command like:

	int <= searchValue1 || ( float > searchValue2 && string == searchValue3)

## Query definition

Query definition is quite simple. There is for example no special attribute necessary to activate (correct) evaluation on the delta store. The necessary fields are:

- type: fixed and always *ExpressionTableScan*
- expression: the expression you want to execute, in the exact same way it was defined
- columns: an array of the columns which should be evaluated
- values: an array for the values to be used for comparison


		“query“ : {
			“type" : "ExpressionTableScan",
           	"expression" : "EQ_INT_AND_EQ_FLOAT_AND_EQ_STRING",
           	"columns" : [0, 2, 4],
           	"values" : [3, 18.2, "SAP AG"]
		}

**Important**: columns and values have to be an array even if you are only checking for one column

## The workflow
1. Define expressions in expressionsToGenerate.txt
1. trigger global make process
1. use expression in query


**Alternatively:** It is also possible to trigger the generation process manually by calling *python generator.py* from this directory.