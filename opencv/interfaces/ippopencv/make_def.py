import sys, os, string, re
tmp_name = "tmp.h"

os.system( "cpp opencvipp_funclist.h > " + tmp_name );
f = open( tmp_name, "r" )
ll = f.readlines()
f.close()
os.remove( tmp_name )

f = open( "export.def", "w" )
print >>f, \
"""; The file has been generated automatically from opencvipp_funclist.h.
; Do not alter it!
   
EXPORTS"""

fn_regexp = re.compile( r"IPPAPI\s*\(\s*IppStatus,\s*(\w+)" )

for l in ll:
    fn_list = fn_regexp.findall(l)
    for fn in fn_list:
        print >> f, "    " + fn

f.close()
        

