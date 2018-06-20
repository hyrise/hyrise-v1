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

# We have Dfferent dicts for main and delta operators due to the fact
# that LT and GT comparisons are executed on valueIds on Main.
# getValueIdForValueGreater/Smaller is used to retrieve the valueId
# for the closest value in the dictionary in case the actual value
# is not part of the dictionary. Since the closest valueId does not
# represent the actual value properly because it is greater/smaller
# the use of "<" or ">" will deliver wrong results.
#
# Example:
# Expression: columnA < 4
# Operator: LT (<)
# Table: | columnA |
#		 	  1 => valueId: 0
#			  2 => valueId: 1
#			  3 => valueId: 2
#
# getValueIdForValueSmaller(4) would return 2. The evaluation would
# look like valueId(every value of columnA) < 2 which would only
# return the values 1 and 2. Value 3 would be excluded because it
# was used as the closest value to value 4 which is not part of
# the dictionary. If we change the comparison operator now to "<="
# the result will contain the values 1, 2, 3 and hence be correct.
#
# This does not apply for evaluations on delta because they are
# executed on actual values, not IDs.


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
import shutil
import os

COMMENT_SYMBOL = "#"

#cleanup expression directory if necessary and rebuild
if os.path.isdir("generatedExpressions"):
	shutil.rmtree("generatedExpressions")
os.mkdir("generatedExpressions")

try:
	expressionsToGenerateFile = open("./expressionsToGenerate.txt", "r")
except:
	print "Looking for file expressionsToGenerate.txt in expressionGeneration directory"
	sys.exit()
expressionToGenerate = expressionsToGenerateFile.readline().rstrip("\n")

numberOfExpressions = 0

while expressionToGenerate != '':

	#if line is a comment jump to next one
	if (expressionToGenerate[0] == COMMENT_SYMBOL):
		expressionToGenerate = expressionsToGenerateFile.readline().rstrip("\n")
		continue

	expression = Expression(expressionToGenerate)

	expressionSplit = expressionToGenerate.split("_")

	expression.numberOfColumns = 0

	for expressionPart in expressionSplit:
		if expressionPart in dataTypeDict:
			expression.appendDatatype(expressionPart)

	for expressionPart in expressionSplit:
		if expressionPart in comparisonOperatorDict:
			expression.evaluationString += "_mainVector[{numberOfColumn}]->getRef(_columns[{numberOfColumn}], currentRow) {expressionPart} valueIds[{numberOfColumn}]"
			if expressionPart == "EQ":
				expression.evaluationStringDelta += "_deltaVector[{numberOfColumn}]->getRef(_columns[{numberOfColumn}], currentRow) {expressionPart} valueId{numberOfEQComparisons}"
				expression.numberOfEQComparisons += 1
			else :
				expression.evaluationStringDelta += "_deltaDictionary{numberOfColumn}->getValueForValueId(_deltaVector[{numberOfColumn}]->getRef(_columns[{numberOfColumn}], currentRow)) {expressionPart} _value{numberOfColumn}"

			expression.evaluationString = expression.evaluationString.format(numberOfColumn = expression.numberOfColumns, expressionPart = comparisonOperatorMainDict[expressionPart])
			expression.evaluationStringDelta = expression.evaluationStringDelta.format(numberOfColumn = expression.numberOfColumns, numberOfEQComparisons = expression.numberOfEQComparisons - 1, expressionPart = comparisonOperatorDict[expressionPart])
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

	numberOfExpressions += 1

logText = "Successfully generated {0} expressions\n".format(numberOfExpressions)
open("./generatedExpressions/generationLogFile.txt", "w").write(logText)