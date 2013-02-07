import os
import sys
from fnmatch import fnmatch
from collections import defaultdict
from cpplint import _STL_HEADERS, _CPP_HEADERS, _ClassifyInclude, FileInfo, _RE_PATTERN_INCLUDE_NEW_STYLE, _RE_PATTERN_INCLUDE

_C_SYS_HEADER = 1
_CPP_SYS_HEADER = 2
_LIKELY_MY_HEADER = 3
_POSSIBLE_MY_HEADER = 4
_OTHER_HEADER = 5
_OTHER_HEADER_EXT = 6
_OTHER_HEADER_INT = 7
_MY_HEADER = 8

_TYPE_NAMES = {
      _C_SYS_HEADER: 'C system header',
      _CPP_SYS_HEADER: 'C++ system header',
      _LIKELY_MY_HEADER: 'header this file implements',
      _POSSIBLE_MY_HEADER: 'header this file may implement',
      _OTHER_HEADER: 'other header',
      _OTHER_HEADER_EXT: 'external header',
      _OTHER_HEADER_INT: 'project header'
      }

_HEADER_EXTENSIONS = frozenset(['h', 'hpp', 'hxx'])
_IMPL_EXTENSIONS = frozenset(['cc', 'cpp', 'cxx'])


_C_SYS_HEADERS = ["assert.h", "complex.h", "ctype.h", "errno.h", "fenv.h", 
                  "inttypes.h", "iso646.h", "limits.h", "locale.h", "math.h", 
                  "setjmp.h", "signal.h", "stdalign.h", "stdarg.h", "stdatomic.h",
                  "stdbool.h", "stddef.h", "stdint.h", "stdio.h", "stdlib.h",
                  "stdnoreturn.h", "string.h", "tgmath.h", "threads.h", "time.h",
                  "uchar.h", "wchar.h", "wctype.h", "unistd.h", "sys/*", "dirent.h"]


_HEADER = 0
_IMPLEMENTATION = 1
_UNKNOWN = 2

_SYS_STYLE = '#include <%s>'
_LOC_STYLE = '#include "%s"'

_ORDER = [ (_MY_HEADER, _LOC_STYLE), 
           (_C_SYS_HEADER, _SYS_STYLE),
           (_CPP_SYS_HEADER, _SYS_STYLE),
           (_OTHER_HEADER, _LOC_STYLE),
           (_OTHER_HEADER_EXT, _SYS_STYLE),
           (_OTHER_HEADER_INT, _LOC_STYLE) ]



_EXTERNAL_LIBS = [
    "boost/**",
    "json.h",
    "llvm/**",
    "log4cxx/**",
    ]

def classify_file(filename):
    ext = os.path.splitext(filename)[1]
    if ext in _HEADER_EXTENSIONS: return _HEADER
    elif ext in _IMPL_EXTENSIONS: return _IMPLEMENTATION
    else: return _UNKNOWN


def classify_include(filename, include):
    print include, any(fnmatch(include, pattern) for pattern in _EXTERNAL_LIBS)
    if any(fnmatch(include, pattern) for pattern in _EXTERNAL_LIBS):
        return _OTHER_HEADER_EXT, include
    if include in _CPP_HEADERS or include in _STL_HEADERS:
        return _CPP_SYS_HEADER, include
    if any(fnmatch(include, pattern) for pattern in _C_SYS_HEADERS):
        return _C_SYS_HEADER, include
    
    split_path = filename.split("/")
    split_module = include.split("/")
    if len(split_path) <= 3:
        return _OTHER_HEADER_INT, include

    current_module = split_path[-2]
    include_has_module = len(split_module) == 2

    if not include.startswith(current_module) and not include_has_module:
        include = current_module+"/"+include

    include_wo_ext = os.path.splitext(include)[0]
    if include_wo_ext in filename:
        return _MY_HEADER, include
    return _OTHER_HEADER_INT, include

def process_includes(filename):
    lines =  open(filename).readlines()
    includes = defaultdict(lambda: set())
    code_lines = []
    first_include_line = None
    found_macro_between_includes = None
    for lineno, line in enumerate(lines):
        match = _RE_PATTERN_INCLUDE.search(line)
        if match:
            if first_include_line is None:
                first_include_line = lineno
                code_lines.append("#REPLACE ME LATER")
            else:
                if found_macro_between_includes:
                    print "Backing off from", filename
                    return
            include = match.group(2)
            is_system = (match.group(1) == '<')
            include_type, include = classify_include(filename, include)
            includes[include_type].add(include)
        else:
            if first_include_line and not found_macro_between_includes and line.startswith("#"):
                found_macro_between_includes = True
            code_lines.append(line)

    if first_include_line is None:
        print "No includes in", filename
        return

    file_type = classify_file(filename)

    result = "" 
    for order, style in _ORDER:
        for include in sorted(includes[order]):
            result += style % include + "\n"
        if includes[order]:
            result += "\n"
    code_lines[first_include_line] = result
    space_to_code = first_include_line+1
    while space_to_code < len(code_lines) and code_lines[space_to_code].strip() == "":
        space_to_code += 1
    code_lines = code_lines[:first_include_line+1] + code_lines[space_to_code:]

    open(filename, "w").writelines(code_lines)

def main(filenames):
    for filename in filenames:
        process_includes(filename)

if __name__=="__main__":
    main(sys.argv[1:])


