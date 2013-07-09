DOXYFILE = tools/Doxyfile
doxygen:
	doxygen $(DOXYFILE)

docs: doxygen 
	$(MAKE) --directory=docs/ html

docs_fast: DOXYFILE := tools/FastDoxyfile
docs_fast: docs 

docs_dash: DOXYFILE := tools/DashDoxyfile
docs_dash: docs
