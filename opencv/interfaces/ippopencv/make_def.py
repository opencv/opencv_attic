import sys, os, string, re
tmp_name = "tmp.h"

ipp_ver = 4

if len(sys.argv) == 2 and sys.argv[1][0] in "0123456789":
    ipp_ver = float(sys.argv[1])

iver = int(round(ipp_ver*100))

os.system( "cpp -DIPP=%d opencvipp_funclist.h > %s" % (iver, tmp_name) );
f = open( tmp_name, "r" )
ll = f.readlines()
f.close()
os.remove( tmp_name )

if iver % 100 == 0:
    def_filename = "export%d.def" % (iver/100,)
else:
    def_filename = "export%d.def" % (iver/10,)

f = open( def_filename, "w" )
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
