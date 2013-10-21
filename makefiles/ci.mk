.PHONY: ci_steps
ci_steps: coverage.xml sloccount.sc simian.xml

coverage.xml: ci_test
	./tools/gcovr -x -r `pwd` -e '.*/src/bin/' -e '.*/third_party/' > coverage.xml

sloccount.sc:
	sloccount --wide --details src/ | grep -v src/lib/SQL/parser > sloccount.sc

simian.xml:
	-java -jar third_party/simian/simian-2.3.33.jar -excludes="**/units_*/*" -formatter=xml:simian.xml "src/**.cpp" "src/**.h" "src/**.hpp"
