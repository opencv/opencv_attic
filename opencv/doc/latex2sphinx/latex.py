import sys
from pyparsing import Word, CharsNotIn, Optional, OneOrMore, ZeroOrMore, Group, Forward, ParseException, Literal, Suppress, replaceWith, StringEnd, lineno, QuotedString

class Argument:
    def __init__(self, s, loc, toks):
        self.str = toks[1]
    def __repr__(self):
        return "[%s]" % self.str
    def __str__(self):
        return self.str
def argfun(s, loc, toks):
    return Argument(s, loc, toks)

class Parameter:
    def __init__(self, s, loc, toks):
        self.str = toks[0].asList()
    def __repr__(self):
        return '{' + "".join([str(s) for s in self.str]) + '}'
        return "{%s}" % self.str
    def __str__(self):
        return "".join([str(s) for s in self.str])
def paramfun(s, loc, toks):
    return Parameter(s, loc, toks)

class TexCmd:
    def __init__(self, s, loc, toks):
        self.cmd = str(toks[0])[1:]
        #print 'cmd', self.cmd
        self.args = toks[1].asList()
        self.params = toks[2].asList()
        self.lineno = lineno(loc, s)
        self.filename = None
    def __repr__(self):
        return '\\' + self.cmd + "".join([repr(a) for a in self.args]) + "".join([repr(p) for p in self.params])

class ZeroOrMoreAsList(ZeroOrMore):
    def __init__(self, *args):
        ZeroOrMore.__init__(self, *args)
        def listify(s, loc, toks):
            return [toks]
        self.setParseAction(listify)

backslash = chr(92)

texcmd = Forward()
filler = CharsNotIn(backslash + '\n')
filler2 = CharsNotIn(backslash + '\n' + '}')

arg = '[' + CharsNotIn("]") + ']'
arg.setParseAction(argfun)
param = Suppress(Literal('{')) + ZeroOrMoreAsList(filler2 | texcmd) + Suppress(Literal('}'))
param.setParseAction(paramfun)
texcmd << Word("\\", "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789[]") + ZeroOrMoreAsList(arg) + ZeroOrMoreAsList(param)
def texcmdfun(s, loc, toks):
    if str(toks[0])[1:] == 'input':
        filename = "../" + toks[2].asList()[0].str[0] + ".tex"
        print 'Now parsing', filename, loc
        return parsefile(filename, lineno(loc, s))
    else:
        return TexCmd(s, loc, toks)
texcmd.setParseAction(texcmdfun)

#legal = "".join([chr(x) for x in set(range(32, 127)) - set(backslash)])
#filler = Word(legal)
document = ZeroOrMore(texcmd | filler) + StringEnd().suppress()

if 0:
    print document.parseString("(represented as \\cvCppCross{Mat}'s)")
    print document.parseString("(represented as \\cvCppCross{Mat \\foo{bar}}'s)")
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

    docstr = "".join(lines).replace('\\_', '_')
    # document.setFailAction(None)
    try:
        r = document.parseString(docstr)
        for x in r:
            if isinstance(x, TexCmd) and not x.filename:
                x.filename = filename
        return r
    except ParseException, pe:
        print 'Fatal problem at %s line %d col %d' % (filename, pe.lineno, pe.col)
        print pe.line
        sys.exit(1)

fulldoc = parsefile(sys.argv[1], 0)

# Filter on target language
def preprocess_conditionals(fd, conditionals):
    r = []
    ifstack = []
    for x in fd:
        if isinstance(x, TexCmd):
            ll = x.cmd.rstrip()
            loc = (x.filename, x.lineno)
            if ll.startswith("if"):
                # print " " * len(ifstack), '{', loc
                ifstack.append((conditionals.get(ll[3:], False), loc))
            elif ll.startswith("else"):
                ifstack[-1] = (not ifstack[-1][0], ifstack[-1][1])
            elif ll.startswith("fi"):
                ifstack.pop()
                # print " " * len(ifstack), '}', loc
            elif not False in [p for (p,_) in ifstack]:
                r.append(x)
        else:
            if not False in [p for (p,_) in ifstack]:
                r.append(x)
    if ifstack != []:
        print "unterminated if", ifstack
        sys.exit(0)
    return r

language = 'py'
doc = preprocess_conditionals(fulldoc, {
                                      'C' : language=='c',
                                      'Python' : language=='py',
                                      'Py' : language=='py',
                                      'CPy' : (language=='py' or language == 'c'),
                                      'Cpp' : language=='cpp',
                                      'plastex' : True})

raw = open('raw', 'w')
for x in doc:
    print >>raw, repr(x)
raw.close()

class SphinxWriter:
    def __init__(self, filename):
        self.f_index = open(filename, 'wt')
        self.f = self.f_index
        self.f_chapter = None
        self.f_section = None
        self.indent = 0
        self.state = None
        self.envstack = []
        self.tags = open('TAGS', 'w')

    def write(self, s):
        self.f.write(s.replace('\n', '\n' + self.indent * "    "))

    def doplain(self, s):
        if self.state == 'fpreamble':
            self.description += s
        else:
            print >>self, s

    def docmd(self, c):
        if self.state == 'math':
            if c.cmd != ']':
                self.default_cmd(c)
            else:
                self.indent -= 1
                self.state = None
                print >>self
        else:
            if c.cmd == '[':
                meth = self.cmd_gomath
            else:
                cname = "cmd_" + c.cmd
                meth = getattr(self, cname, self.default_cmd)
            meth(c)

    def cmd_gomath(self, c):
        self.state = 'math'
        print >>self, "\n.. math::"
        self.indent += 1
        print >>self

    def cmd_chapter(self, c):
        filename = str(c.params[0]).lower().replace(' ', '_').replace('/','_')
        self.f_index.write("    %s\n" % filename)
        self.f_chapter = open(filename + '.rst', 'wt')
        self.f_section = None
        self.f = self.f_chapter
        self.indent = 0
        title = str(c.params[0])
        print >>self, '*' * len(title)
        print >>self, title
        print >>self, '*' * len(title)
        print >>self
        print >>self, '.. toctree::'
        print >>self, '    :maxdepth: 2'
        print >>self

    def cmd_section(self, c):
        filename = str(c.params[0]).lower().replace(' ', '_').replace('/','_')
        self.f_chapter.write("    %s\n" % filename)
        self.f_section = open(filename + '.rst', 'wt')
        self.f = self.f_section
        self.indent = 0
        title = str(c.params[0])
        print >>self, title
        print >>self, '=' * len(title)
        print >>self

    def cmd_cvCppCross(self, c):
        print >>self, ":ref:`%s`" % str(c.params[0])
    def cmd_cvCPyCross(self, c):
        print >>self, ":ref:`%s`" % str(c.params[0])

    def cmd_cvCPyFunc(self, c):
        self.indent = 0
        nm = str(c.params[0])
        print >>self, "\n.. index:: %s\n" % nm
        print >>self, ".. _%s:\n" % nm
        print >>self, nm
        print >>self, '-' * len(nm)
        print >>self
        self.state = 'fpreamble'
        self.description = ""
        print >>self.tags, "%s\t%s\t%d" % (nm, c.filename, c.lineno)

    def cmd_cvdefPy(self, c):
        s = str(c.params[0])
        print >>self, ".. function:: " + s + "\n"
        self.indent = 1
        print >>self, self.description
        print >>self
        self.state = None

    def cmd_begin(self, c):
        s = str(c.params[0])
        self.envstack.append((s, (c.filename, c.lineno)))
        if s == 'description':
            pass
        else:
            self.default_cmd(c)

    def cmd_end(self, c):
        s = str(c.params[0])
        if self.envstack == []:
            print "Cannot pop at", (c.filename, c.lineno)
        self.envstack.pop()
        if s == 'description':
            pass
        else:
            self.default_cmd(c)
        
    def cmd_cvdefC(self, c):
        pass
    def cmd_cvdefCpp(self, c):
        pass
    def cmd_cvCpp(self, c):
        pass

    def cmd_cvarg(self, c):
        ee = [n for (n,_) in self.envstack[1:]]
        if ee == ['description']:
            print >>self, '\n:param %s: ' % str(c.params[0]),
        elif ee == ['description', 'description']:
            print >>self, '\n* **%s** ' % str(c.params[0]),
        else:
            print self.envstack
            assert 0
        self.indent += 1
        self.doL(c.params[1].str)
        self.indent -= 1
        print >>self

    def cmd_texttt(self, c):
        print >>self, "``" + str(c.params[0]) + "``"

    def default_cmd(self, c):
        if self.f == self.f_section:
            print >>self, repr(c)

    def doL(self, L):
        for x in L:
            if isinstance(x, TexCmd):
                self.docmd(x)
            else:
                self.doplain(x)

# dststack[0] -> index.rst
#         [1] -> current chapter
#         [2] -> current section


sr = SphinxWriter('index.rst')
print >>sr, """
OpenCV |version| Python Reference
=================================

The OpenCV Wiki is here: http://opencv.willowgarage.com/

Contents:

.. toctree::
    :maxdepth: 2

"""
sr.doL(doc)
sys.exit(0)

structuring = ['chapter', 'section']
for x in doc:
    if isinstance(x, TexCmd):
        if x.cmd in structuring:
            level = structuring.index(x.cmd)
            filename = str(x.params[0]).lower().replace(' ', '_').replace('/','_')
            print >>dststack[level], "   %s" % filename
            dststack[level + 1] = SphinxWriter(filename + ".rst", envstack)
            title = str(x.params[0])
            if level == 0:
                print >>dststack[level + 1], '*' * len(title)
                print >>dststack[level + 1], title
                print >>dststack[level + 1], '*' * len(title)
                print >>dststack[level + 1]
                print >>dststack[level + 1], '.. toctree::'
                print >>dststack[level + 1], '   :maxdepth: 2'
                print >>dststack[level + 1]
            if level == 1:
                print >>dststack[level + 1], title
                print >>dststack[level + 1], '=' * len(title)
                print >>dststack[level + 1]
        else:
            if dststack[level + 1]:
                dststack[level + 1].docmd(x)
    else:
        dststack[level + 1].doplain(x)

print >>dststack[0], """
Indices and tables
==================

* :ref:`genindex`
* :ref:`search`
"""
