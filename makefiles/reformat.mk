reformat: $(PROJECT_ROOT)/tools/clang34
	$(PROJECT_ROOT)/tools/clang34/bin/clang-format -i -style='{BasedOnStyle: Google, BinPackParameters: false, AlignTrailingComments: false, DerivePointerBinding: false, PointerBindsToType: true, MaxEmptyLinesToKeep: 3, ColumnLimit: 120, AllowAllParametersOfDeclarationOnNextLine: false, AllowShortLoopsOnASingleLine: false, AllowShortIfStatementsOnASingleLine: false, ConstructorInitializerAllOnOneLineOrOnePerLine: true}'  `find src/ -type f -regextype posix-awk -regex "(.*\.(h|cpp|cc|hpp))"` 
reformat-check: reformat
	if [ "`git status --porcelain src | wc -l`" = "0" ]; then true; else false; fi
