dataTypeDict = {
	"INT": "hyrise_int_t",
	"FLOAT": "hyrise_float_t",
	"STRING": "hyrise_string_t"
}

jsonParsMethodDict = {
	"INT": "asInt()",
	"FLOAT": "asFloat()",
	"STRING": "asString()"
}

operatorDict = {
	"EQ": "==",
	"LT": "<",
	"GT": ">",
	"LTEQ": "<=",
	"GTEQ": ">=",
	"AND": "&&",
	"OR": "||",
	"(": "(",
	")": ")"
}




class Expression(object):
	def __init__(self, expressionString):
		self.callName = expressionString
		self.name = expressionString.replace("(", "")
		self.name = self.name.replace(")", "")
		self.dataTypes = []
		self.jsonParseMethods = []
		self.operators = []
		self.evaluationString = ""
		self.evaluationStringDelta = ""
		self.numberOfColumns = 4

	def appendDatatype(self, dataType):
		self.dataTypes.append(dataTypeDict[dataType])
		self.appendJsonParseMethod(dataType)

	def appendJsonParseMethod(self, dataType):
		self.jsonParseMethods.append(jsonParsMethodDict[dataType])

	def appendOperator(self, operator):
		self.operators.append(operatorDict[operator])


from jinja2 import Template
from collections import deque

expressionsToGenerateFile = open("./expressionsToGenerate.txt", "r")
expressionToGenerate = expressionsToGenerateFile.readline().rstrip("\n")

i = 0

while expressionToGenerate != '':
	expression = Expression(expressionToGenerate)

	expressionSplit = expressionToGenerate.split("_")

	expression.numberOfColumns = 0

	for expressionPart in expressionSplit:
		if expressionPart == "INT" or expressionPart == "FLOAT" or expressionPart == "STRING":
			expression.appendDatatype(expressionPart)

	for expressionPart in expressionSplit:
		if expressionPart == "EQ" or expressionPart == "LT" or expressionPart == "GT" or expressionPart == "LTEQ" or expressionPart == "GTEQ":
			expression.evaluationString += "_mainVector[" + str(expression.numberOfColumns) + "]->getRef(_columns[" + str(expression.numberOfColumns) + "], currentRow) " + operatorDict[expressionPart] + " valueIdExtended[" + str(expression.numberOfColumns) + "]"
			expression.evaluationStringDelta += "_deltaDictionary" + str(expression.numberOfColumns) + "->getValueForValueId(_deltaVector[" + str(expression.numberOfColumns) + "]->getRef(_columns[" + str(expression.numberOfColumns) + "], currentRow)) " + operatorDict[expressionPart] + " _value" + str(expression.numberOfColumns)
			expression.numberOfColumns += 1
			expression.appendOperator(expressionPart)
		elif expressionPart == "AND" or expressionPart == "OR" or expressionPart == "(" or expressionPart == ")":
			expression.evaluationString += " " + operatorDict[expressionPart] + " "
			expression.evaluationStringDelta += " " + operatorDict[expressionPart] + " "


	expressionHeaderTemplateFile = open("./ExpressionTemplateH.tpl", "r")
	expressionHeaderTemplate = Template(expressionHeaderTemplateFile.read())
	expressionHeaderTemplateFile.close()

	expressionImplTemplateFile = open("./ExpressionTemplateCPP.tpl", "r")
	expressionImplTemplate = Template(expressionImplTemplateFile.read())
	expressionImplTemplateFile.close()

	open("./generatedExpressions/" + expression.name + ".h", "w").write(expressionHeaderTemplate.render(expression = expression))
	open("./generatedExpressions/" + expression.name + ".cpp", "w").write(expressionImplTemplate.render(expression = expression))

	expressionToGenerate = expressionsToGenerateFile.readline().rstrip("\n")