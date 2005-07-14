import sys, os, string, re
tmp_name = "tmp.h"

ipp_ver = 4

if len(sys.argv) == 2 and sys.argv[1] == "5":
    ipp_ver = 5

os.system( "cpp -DIPP%d opencvipp_funclist.h > %s" % (ipp_ver, tmp_name) );
f = open( tmp_name, "r" )
ll = f.readlines()
f.close()
os.remove( tmp_name )

f = open( "export%d.def" % (ipp_ver,) , "w" )
print >>f, \
"""; The file has been generated automatically from opencvipp_funclist.h.
; Do not alter it!
   
EXPORTS"""

fn_regexp = re.compile( r"IPPAPI\s*\([^,]+,\s*(\w+)" )

for l in ll:
    fn_list = fn_regexp.findall(l)
    for fn in fn_list:
        print >> f, "    " + fn

f.close()
        

