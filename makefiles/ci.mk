# Build invocation
ci_test: unit_test_params = --gtest_output=xml:$(subst $(build_dir)/,,$@).xml
ci_test: all $(all_test_binaries)

ci_build_not_parallel: coverage lint sloc duplicates

ci_build: all
	@$(MAKE) $(MAKEFLAGS) ci_build_not_parallel -j 1

hudson_test: ci_build

coverage: ci_test
	./gcovr -x -r `pwd` -e '.*/src/bin/' -e '.*/third_party/' > coverage.xml
