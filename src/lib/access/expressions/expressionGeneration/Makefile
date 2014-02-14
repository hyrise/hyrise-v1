all: generatedExpressions/generationLogFile.txt

generatedExpressions/generationLogFile.txt: expressionsToGenerate.txt ExpressionTemplateCPP.tpl ExpressionTemplateH.tpl generator.py
	python generator.py