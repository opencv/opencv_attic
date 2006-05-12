#! /usr/bin/env python
"""
This script extracts macros #defines from those OpenCV headers that can't be
directly parsed by current SWIG versions and must be pre-filtered by
the C preprocessor (that erases all #defines).  Type information is missing in the 
macros, so C code can't be regenerated.  Instead, try to convert C to Python code.
C macros too complicated to represent in python using regexes are listed in EXCLUDE
"""

# TODO: support multiline macros

import sys, re

# macros with illegal python syntax
EXCLUDE = { 'CV_SWAP':1, 'CV_IABS':1 } 

# force this to be part of cv module
# otherwise becomes cv.cvmacros
print "%module cv"
print "%pythoncode %{"
for fn in sys.argv[1:]:
    f = open( fn, "r" )
    in_define = 0
    for l in f.xreadlines():
        # get rid of pointer member accesses
        (l, n) = re.subn( "->", ".", l)
        # get rid of casts
        (l, n) = re.subn( "\((?:Cv\w+|void|uchar|int)\*\)", "", l)
        # get rid of dereferences
        (l, n) = re.subn( "\(\*\(", "((", l)
        m = re.match( r"^#define\s+((?:CV_|IPL_)\w+)\s*\(([_, a-zA-Z0-9]*)\)\s*(.*)", l )
        if m and not l.endswith( "\\\n" ) and not EXCLUDE.has_key(m.group(1)):
            print "def %s (%s):" % (m.group(1), m.group(2))
            print "\treturn %s\n" % (m.group(3))
    f.close()

print "%}"
