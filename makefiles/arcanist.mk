arc-install:
	mkdir -p tools/arcane
	git clone git://github.com/facebook/libphutil.git tools/arcane/libphutil
	git clone git://github.com/facebook/arcanist.git tools/arcane/arcanist

arc-init:
	PATH=$(PATH):$(IMH_PROJECT_PATH)/tools/arcane/arcanist/bin/ arc install-certificate

arc-help:
	@echo "Add `pwd`/tools/arcane/arcanist/bin/ to env PATH" 
