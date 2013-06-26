test_uncommited: unit_test_params = --minimal
test_uncommited:
	$(MAKE) `python tools/test_uncommited.py`