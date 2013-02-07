lint:	
	-find src/ -regextype posix-awk -regex "(.*\.(h|cpp|cc|hpp))" -type f -exec python tools/cpplint.py --filter=-whitespace/line_length,-whitespace/comments,-whitespace/blank_line,,-build/include_what_you_use,-build/include_order,-whitespace,-readability,-build/include,-runtime/rtti,-legal/copyright,-runtime/references,-runtime/threadsafe_fn {} + 2> cpplint.log


lint_visible:	
	-find src/ -regextype posix-awk -regex "(.*\.(h|cpp|cc|hpp))" -type f -exec python tools/cpplint.py --filter=-whitespace/line_length,-whitespace/comments,-whitespace/blank_line,,-build/include_what_you_use,-build/include_order,-whitespace,-readability,-build/include,-runtime/rtti,-legal/copyright,-runtime/references,-runtime/threadsafe_fn {} +
