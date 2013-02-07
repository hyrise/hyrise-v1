reformat:
	find src/ -regextype posix-awk -regex "(.*\.(h|cpp|cc|hpp))" -type f -exec astyle --options=tools/google_astyle {} +
	find src/ -regextype posix-awk -regex "(.*\.(h|cpp|cc|hpp))" -type f -exec sh tools/format.sh {} \;
	find src/ -regextype posix-awk -regex "(.*\.(h|hpp))" -type f -exec python tools/add_guards.py {} +
#	find src/ -regextype posix-awk -regex "(.*\.(h|cpp|cc|hpp))" -type f -exec python tools/add_copyright.py {} +
#       find src/ -regextype posix-awk -regex "(.*\.(h|cpp|cc|hpp))" -type f -exec python tools/order_includes.py {} +


MASTER_DIFF = git diff master --name-only --diff-filter=AM
reformat_changes_to_master:
	find `$(MASTER_DIFF)` -regextype posix-awk -regex "(.*\.(h|cpp|cc|hpp))" -type f -exec astyle --options=tools/google_astyle {} +
	find `$(MASTER_DIFF)` -regextype posix-awk -regex "(.*\.(h|cpp|cc|hpp))" -type f -exec sh tools/format.sh {} +
	find `$(MASTER_DIFF)` -regextype posix-awk -regex "(.*\.(h|hpp))" -type f -exec python tools/add_guards.py {} +
