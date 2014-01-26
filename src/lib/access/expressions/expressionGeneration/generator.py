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

comparisonOperatorDict = {
	"EQ": "==",
	"LT": "<",
	"GT": ">",
	"LTEQ": "<=",
	"GTEQ": ">="
}

comparisonOperatorMainDict = {
	"EQ": "==",
	"LT": "<=",
	"GT": ">=",
	"LTEQ": "<=",
	"GTEQ": ">="
}

generalOperatorDict = {
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
		self.numberOfEQComparisons = 0

	def appendDatatype(self, dataType):
		self.dataTypes.append(dataTypeDict[dataType])
		self.appendJsonParseMethod(dataType)

	def appendJsonParseMethod(self, dataType):
		self.jsonParseMethods.append(jsonParsMethodDict[dataType])

	def appendOperator(self, operator):
		self.operators.append(comparisonOperatorDict[operator])


from jinja2 import Template
import sys

try:
	expressionsToGenerateFile = open("./expressionsToGenerate.txt", "r")
except:
	print "Looking for file expressionsToGenerate.txt in expressionGeneration directory"
	sys.exit()
expressionToGenerate = expressionsToGenerateFile.readline().rstrip("\n")

i = 0

while expressionToGenerate != '':
	expression = Expression(expressionToGenerate)

	expressionSplit = expressionToGenerate.split("_")

	expression.numberOfColumns = 0

	for expressionPart in expressionSplit:
		if expressionPart in dataTypeDict:
			expression.appendDatatype(expressionPart)

	for expressionPart in expressionSplit:
		if expressionPart in comparisonOperatorDict:
			expression.evaluationString += "_mainVector[" + str(expression.numberOfColumns) + "]->getRef(_columns[" + str(expression.numberOfColumns) + "], currentRow) " + comparisonOperatorMainDict[expressionPart] + " valueIds[" + str(expression.numberOfColumns) + "]"
			if expressionPart == "EQ":
				expression.evaluationStringDelta += "_deltaVector[" + str(expression.numberOfColumns) + "]->getRef(_columns[" + str(expression.numberOfColumns) + "], currentRow) " + comparisonOperatorDict[expressionPart] + " valueId" + str(expression.numberOfEQComparisons)
				expression.numberOfEQComparisons += 1
			else :
				expression.evaluationStringDelta += "_deltaDictionary" + str(expression.numberOfColumns) + "->getValueForValueId(_deltaVector[" + str(expression.numberOfColumns) + "]->getRef(_columns[" + str(expression.numberOfColumns) + "], currentRow)) " + comparisonOperatorDict[expressionPart] + " _value" + str(expression.numberOfColumns)
			expression.numberOfColumns += 1
			expression.appendOperator(expressionPart)
		elif expressionPart in generalOperatorDict:
			expression.evaluationString += " " + generalOperatorDict[expressionPart] + " "
			expression.evaluationStringDelta += " " + generalOperatorDict[expressionPart] + " "


	expressionHeaderTemplateFile = open("./ExpressionTemplateH.tpl", "r")
	expressionHeaderTemplate = Template(expressionHeaderTemplateFile.read())
	expressionHeaderTemplateFile.close()

	expressionImplTemplateFile = open("./ExpressionTemplateCPP.tpl", "r")
	expressionImplTemplate = Template(expressionImplTemplateFile.read())
	expressionImplTemplateFile.close()

	open("./generatedExpressions/" + expression.name + ".h", "w").write(expressionHeaderTemplate.render(expression = expression))
	open("./generatedExpressions/" + expression.name + ".cpp", "w").write(expressionImplTemplate.render(expression = expression))

	expressionToGenerate = expressionsToGenerateFile.readline().rstrip("\n")