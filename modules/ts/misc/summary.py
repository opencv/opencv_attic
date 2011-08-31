import testlog_parser, sys, os, xml
from table_formatter import *
from optparse import OptionParser

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print "Usage:\n", os.path.basename(sys.argv[0]), "<log_name1>.xml [<log_name2>.xml ...]"
        exit(0)
        
    parser = OptionParser()
    parser.add_option("-o", "--output", dest="format", help="output results in text format (can be 'txt', 'html' or 'auto' - default)", metavar="FMT", default="auto")
    parser.add_option("-m", "--metric", dest="metric", help="output metric", metavar="NAME", default="gmean")
    parser.add_option("-u", "--units", dest="units", help="units for output values (s, ms (default), mks, ns or ticks)", metavar="UNITS", default="ms")
    (options, args) = parser.parse_args()
    
    options.generateHtml = detectHtmlOutputType(options.format)
    if options.metric not in metrix_table:
        options.metric = "gmean"
        
    # read all passed files
    test_sets = []
    for arg in args:
        try:
            tests = testlog_parser.parseLogFile(arg)
            if tests:
                test_sets.append((arg, tests))
        except IOError as err:
            sys.stderr.write("IOError reading \"" + arg + "\" - " + str(err) + os.linesep)
        except xml.parsers.expat.ExpatError as err:
            sys.stderr.write("ExpatError reading \"" + arg + "\" - " + str(err) + os.linesep)
            
    if not test_sets:
        sys.stderr.write("Error: no test data found" + os.linesep)
        quit()
            
    # find matches
    setsCount = len(test_sets)
    test_cases = {}
    
    for i in range(setsCount):
        for case in test_sets[i][1]:
            name = str(case)
            if name not in test_cases:
                test_cases[name] = [None] * setsCount
            test_cases[name][i] = case
            
    # build table
    getter = metrix_table[options.metric][1] 
    tbl = table(metrix_table[options.metric][0])
    
    # header
    tbl.newColumn("name", "Name of Test", align = "left")
    i = 0
    for set in test_sets:
        tbl.newColumn(str(i), set[0], align = "center")
        i += 1
        
    # rows
    for name in sorted(test_cases.iterkeys()):
        cases = test_cases[name]
        tbl.newRow()
        tbl.newCell("name", name)
        for i in range(setsCount):
            case = cases[i]
            if case is None:
                tbl.newCell(str(i), "-")
            else:
                status = case.get("status")
                if status != "run":
                    tbl.newCell(str(i), status, color = "red")
                else:
                    val = getter(case, cases[0], options.units)
                    if val:
                        tbl.newCell(str(i), "%.3f %s" % (val, options.units), val)
                    else:
                        tbl.newCell(str(i), "-")
            
    # output table
    if options.generateHtml:
        htmlPrintHeader(sys.stdout, "Summary report for %s test logs" % setsCount)
        tbl.htmlPrintTable(sys.stdout)
        htmlPrintFooter(sys.stdout)
    else:
        tbl.consolePrintTable(sys.stdout)
