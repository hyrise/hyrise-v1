sloc:
	sloccount --wide --details src/ | grep -v src/lib/SQL/parser > sloccount.sc

stat:
	cloc --exclude-dir=docs,third_party,tools,build,tbb,test,SQL src
