import sys, re, os.path

class tblCell(object):
    def __init__(self, text, value = None, props = None):
        self.text = text
        self.value = value
        self.props = props

class tblColumn(object):
    def __init__(self, caption, title = None, props = None):
        self.text = caption
        self.title = title
        self.props = props
        
        lines = caption.splitlines()
        self.minWidth = max(max([len(line) for line in lines]), 1)
        self.linesNum = len(lines)
        
class tblRow(object):
    def __init__(self, colsNum, props = None):
        self.maxHeight = 1
        self.cells = [None] * colsNum
        self.props = props

class table(object):
    def_align = "left"
    def_valign = "top"
    def_color = None
    def_colspan = 1
    def_rowspan = 1
    def_bold = False
    def_italic = False
    def_text="-"

    def __init__(self):
        self.columns = {}
        self.rows = []
        self.ridx = -1;
        pass

    def newRow(self, **properties):
        if len(self.rows) - 1 == self.ridx:
            self.rows.append(tblRow(len(self.columns), properties))
        else:
            self.rows[ridx + 1].props = properties
        self.ridx += 1

    def newColumn(self, name, caption, title = None, **properties):
        if name in self.columns:
            index = self.columns[name].index
        else:
            index = len(self.columns)
        if isinstance(caption, tblColumn):
            caption.index = index
            self.columns[name] = caption
            return caption
        else:
            col = tblColumn(caption, title, properties)
            col.index = index
            self.columns[name] = col
            return col

    def getColumn(self, name):
        if isinstance(name, str):
            return self.columns.get(name, None)
        else:
            vals = [v for v in self.columns.values() if v.index == name]
            if vals:
                return vals[0]
        return None

    def newCell(self, col_name, text, value = None, **properties):
        if self.ridx < 0:
            self.newRow()
        col = self.getColumn(col_name)
        row = self.rows[self.ridx]
        if not col:
            return None
        if isinstance(text, tblCell):
            cl = text
        else:
            cl = tblCell(text, value, properties)
        lines = cl.text.splitlines()
        width = max([len(line) for line in lines])
        height = len(lines)
        cl.linesNum = height
        colspan = self.getValue("colspan", cl)
        if colspan < 2:
            if col.minWidth < width:
                col.minWidth = width
        else:
            columns = self.columns.values()
            columns.sort(key=lambda c: c.index)
            columns = columns[col.index: col.index + colspan]
            self.adjustColWidth(columns, width)
            
        if row.maxHeight < height:
            row.maxHeight = height
        row.cells[col.index] = cl
        return cl
    
    def adjustColWidth(self, cols, width):
        total = sum([c.minWidth for c in cols])
        if total + len(cols) - 1 >= width:
            return
        budget = width - len(cols) + 1 - total
        spent = 0
        s = 0
        for col in cols:
            s += col.minWidth
            addition = s * budget / total - spent
            spent += addition
            col.minWidth += addition

    def getValue(self, name, *elements):
        for el in elements:
            try:
                return getattr(el, name)
            except AttributeError:
                pass
            try:
                val = el.props[name]
                if val:
                    return val
            except AttributeError:
                pass
            except KeyError:
                pass
        try:
            return getattr(self.__class__, "def_" + name)
        except AttributeError:
            return None
        
    def consolePrintTable(self):
        columns = self.columns.values()
        columns.sort(key=lambda c: c.index)
        
        headerRow = tblRow(len(columns), {"align": "center", "valign": "top", "bold": True})
        headerRow.cells = columns
        headerRow.maxHeight = max([col.linesNum for col in columns])
        
        self.consolePrintRow(headerRow, columns, -1)
        
        for i in range(0, len(self.rows)):
            self.consolePrintRow(self.rows[i], columns, i)
                
    def consolePrintRow(self, r, columns, idx):
        cells = r.cells
        ln = 0
        while (ln < r.maxHeight):
            i = 0
            while (i < len(cells)):
                cell = cells[i]
                col = columns[i]
                colspan = self.getValue("colspan", cell)
                if colspan == 1:
                    width = col.minWidth
                else:
                    width = colspan - 1 + sum([v.minWidth for v in columns[i: i + colspan]])
                                #for v columns[i:i]
                if cell is None:
                    valign = "middle"
                else:
                    valign = self.getValue("valign", cell, r, col)
                
                if valign == "bottom":
                    lineIdx = cell.linesNum + ln - r.maxHeight
                elif valign == "middle":
                    if cell is None:
                        lineIdx = (ln == (r.maxHeight - 1) / 2)
                    else:
                        lineIdx = ln + (cell.linesNum - r.maxHeight + 1)/2
                else:
                    lineIdx = ln
                                
                self.consolePrintCell(cell, col, r, width, lineIdx,  i > 0)
                i += colspan
            print
            ln += 1
        
    def consolePrintCell(self, cell, col, row, width, lineNum, addSpace):
        text = self.getValue("text", cell)
        if cell is None:
            align = "center"
            if lineNum:
                line = text
            else:
                line = ""
        else:
            if isinstance(cell, tblColumn):
                align = self.getValue("align", row, col)
            else:
                align = self.getValue("align", cell, row, col)
            if lineNum < 0 or lineNum >= cell.linesNum:
                line = ""
            else:
                line = text.splitlines()[lineNum]
        if align == "right":
            pattern = "%" + str(width) + "s"
        elif align == "center":
            pattern = "%" + str((width - len(line)) / 2 + len(line)) + "s" + " " * (width - len(line) - (width - len(line)) / 2)
        else:
            pattern = "%-" + str(width) + "s"
        if addSpace:
            pattern = " " + pattern
        sys.stdout.write(pattern % line,)
        

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print "Usage:\n", os.path.basename(sys.argv[0]), "<log_name>.xml"
        exit(0)

    tbl = table()
    tbl.newColumn("first", "qqqq", align = "left")
    tbl.newColumn("second", "wwww\nz\nx\n")
    tbl.newColumn("third", "wwasdas")

    tbl.newCell(0, "ccc111", align = "right")
    tbl.newCell(1, "dddd1")
    tbl.newCell(2, "8768756754")
    tbl.newRow()
    tbl.newCell(0, "1\n2\n3\n4\n5\n6\n7", align = "center", colspan = 2)
    tbl.newCell(2, "xxx\nqqq", align = "center", colspan = 1, valign = "middle")
    tbl.newRow()
    tbl.newCell(0, "vcvvbasdsadassdasdasv", align = "right", colspan = 2)
    tbl.newRow()
    tbl.newCell(0, "vcvvbv")
    tbl.newCell(1, "3445324", align = "right")
    #tbl.newCell(1, "0000")

    #print vars(tbl)
    
    tbl.consolePrintTable()


    print "!!!!!", tbl.getValue("align", {},  tbl.getColumn(0))

    import testlog_parser

    for arg in sys.argv[1:]:
        print "Tests found in", arg
        tests = testlog_parser.parseLogFile(arg)
        tbl = table()
        tbl.newColumn("name", "Name\nof\nTest", align = "left")
        tbl.newColumn("gmean", "Geometric mean\n(ms)", align = "center")
        
        for t in sorted(tests):
            tbl.newRow()
            gmean = t.get("gmean")
            tbl.newCell("name", str(t))
            if gmean:
                tbl.newCell("gmean", "%.3f ms" % gmean, gmean)
            #t.dump()
        print
        
        tbl.consolePrintTable()
