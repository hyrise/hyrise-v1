import sys

from cpplint import GetHeaderGuardCPPVariable

def UpdateHeaderGuard(filename):
  """From cpplint.py"""
  cppvar = GetHeaderGuardCPPVariable(filename)
  ifndef = None
  ifndef_linenum = 0
  define = None
  define_linenum = 0
  endif = None
  endif_linenum = 0
  lines = open(filename).readlines()
  for linenum, line in enumerate(lines):
    linesplit = line.split()
    if len(linesplit) >= 2:
      # find the first occurrence of #ifndef and #define, save arg
      if not ifndef and linesplit[0] == '#ifndef':
        # set ifndef to the header guard presented on the #ifndef line.
        ifndef = linesplit[1]
        ifndef_linenum = linenum
      if not define and linesplit[0] == '#define':
        define = linesplit[1]
        define_linenum = linenum
    # find the last occurrence of #endif, save entire line
    if line.startswith('#endif'):
      endif = line
      endif_linenum = linenum

  if not all([ifndef, define, endif]):
     return

  print "Updating header in", filename
  lines[ifndef_linenum] = "#ifndef "+cppvar+"\n"
  lines[define_linenum] = "#define "+cppvar+"\n"
  lines[endif_linenum] = "#endif  // "+cppvar+"\n"
  with open(filename, "w") as f: f.writelines(lines)  

def main(filenames):
    for filename in filenames:
        UpdateHeaderGuard(filename)


if __name__=="__main__":
    main(sys.argv[1:])
