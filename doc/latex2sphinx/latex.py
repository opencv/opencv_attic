import sys
from pyparsing import Word, CharsNotIn, Optional, OneOrMore, ZeroOrMore, Group, ParseException, Literal, replaceWith, StringEnd, lineno, QuotedString

class Argument:
    def __init__(self, s, loc, toks):
        self.str = toks[1]
    def __repr__(self):
        return "[%s]" % self.str
def argfun(s, loc, toks):
    return Argument(s, loc, toks)

class Parameter:
    def __init__(self, s, loc, toks):
        self.str = toks[0]
    def __repr__(self):
        return "{%s}" % self.str
def paramfun(s, loc, toks):
    return Parameter(s, loc, toks)

class TexCmd:
    def __init__(self, s, loc, toks):
        self.cmd = str(toks[0])[1:]
        #print 'cmd', self.cmd
        self.args = toks[1].asList()
        self.params = toks[2].asList()
    def __repr__(self):
        return self.cmd + "".join([repr(a) for a in self.args]) + "".join([repr(p) for p in self.params])

class ZeroOrMoreAsList(ZeroOrMore):
    def __init__(self, *args):
        ZeroOrMore.__init__(self, *args)
        def listify(s, loc, toks):
            return [toks]
        self.setParseAction(listify)

arg = '[' + CharsNotIn("]") + ']'
arg.setParseAction(argfun)
# param = '{' + Optional(CharsNotIn("}")) + '}'
param = QuotedString('{', endQuoteChar = '}', multiline = True)
param.setParseAction(paramfun)
texcmd = Word("\\", "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") + ZeroOrMoreAsList(arg) + ZeroOrMoreAsList(param)
def texcmdfun(s, loc, toks):
    if str(toks[0])[1:] == 'input':
        filename = "../" + toks[2].asList()[0].str + ".tex"
        print 'Now parsing', filename, loc
        return parsefile(filename, lineno(loc, s))
    else:
        return TexCmd(s, loc, toks)
texcmd.setParseAction(texcmdfun)

backslash = chr(92)
legal = "".join([chr(x) for x in set(range(32, 127)) - set(backslash)])
filler = CharsNotIn(backslash)
filler = Word(legal)
document = ZeroOrMore(texcmd | filler) + StringEnd().suppress()

if 0:
    print document.parseString('\\hello{foo}')
    sys.exit(0)

def parsefile(filename, startline):
    f = open(filename, "rt")

    lines = list(f)
    def uncomment(s):
        if '%' in s:
            return s[:s.index('%')] + '\n'
        else:
            return s

    lines = [uncomment(l) for l in lines]
    print len(lines), "lines"

    docstr = "".join(lines)
    # document.setFailAction(None)
    try:
        return document.parseString(docstr)
    except ParseException, pe:
        print 'Fatal problem at %s line %d col %d' % (filename, pe.lineno, pe.col)
        print pe.line
        sys.exit(1)

doc = parsefile(sys.argv[1], 0)

raw = open('raw', 'w')
for x in doc:
    print >>raw, repr(x)
raw.close()

for x in doc:
    if isinstance(x, TexCmd):
        if x.cmd == 'chapter':
            print repr(x)
        if x.cmd == 'section':
            print ' ' * 4, repr(x)
