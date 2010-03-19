import sys
from latexparser import latexparser, TexCmd
import distutils.dep_util
import os
import cPickle as pickle
import pyparsing as pp
import StringIO
from qfile import QOpen

sources = ['../' + f for f in os.listdir('..') if f.endswith('.tex')]
if distutils.dep_util.newer_group(["latexparser.py"] + sources, "pickled"):
    fulldoc = latexparser(sys.argv[1], 0)
    pickle.dump(fulldoc, open("pickled", 'wb'))
    raw = open('fulldoc', 'w')
    for x in fulldoc:
        print >>raw, repr(x)
    raw.close()
else:
    fulldoc = pickle.load(open("pickled", "rb"))

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
                ifstack.append((conditionals.get(ll[2:], False), loc))
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
        self.f_index = QOpen(filename, 'wt')
        self.f = self.f_index
        self.f_chapter = None
        self.f_section = None
        self.indent = 0
        self.state = None
        self.envstack = []
        self.tags = []
        self.errors = open('errors', 'wt')
        self.unhandled_commands = set()
        self.freshline = True

    def write(self, s):
        self.freshline = len(s) > 0 and (s[-1] == '\n')
        self.f.write(s.replace('\n', '\n' + self.indent * "    "))

    def appendspace(self):
        """ append a space to the output - if we're not at the start of a line """
        if not self.freshline:
            self.write(' ')

    def doplain(self, s):
        if (len(s) > 1) and (s[0] == '$' and s[-1] == '$') and self.state != 'math':
            s = ":math:`%s`" % s[1:-1]
        elif self.state != 'math':
            s.replace('\\_', '_')
        if self.state == 'fpreamble':
            self.description += s
        else:
            self.write(s)

    def docmd(self, c):
        if self.state == 'math':
            if c.cmd != ']':
                self.default_cmd(c)
            else:
                self.indent -= 1
                self.state = None
                self.write('\n\n')
        else:
            if c.cmd == '[':
                meth = self.cmd_gomath
            else:
                cname = "cmd_" + c.cmd
                meth = getattr(self, cname, self.unrecognized_cmd)
            meth(c)

    def cmd_gomath(self, c):
        self.state = 'math'
        print >>self, "\n\n.. math::"
        self.indent += 1
        print >>self

    def cmd_chapter(self, c):
        filename = str(c.params[0]).lower().replace(' ', '_').replace('/','_')
        self.f_index.write("    %s\n" % filename)
        self.f_chapter = QOpen(filename + '.rst', 'wt')
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
        self.f_section = QOpen(filename + '.rst', 'wt')
        self.f = self.f_section
        self.indent = 0
        title = str(c.params[0])
        print >>self, title
        print >>self, '=' * len(title)
        print >>self

    def cmd_subsection(self, c):
        print >>self, str(c.params[0])
        print >>self, '-' * len(str(c.params[0]))
        print >>self
        self.function_props = {}

    def cmd_includegraphics(self, c):
        filename = '../' + str(c.params[0])
        if not os.path.isfile(filename):
            print >>self.errors, "WARNING: missing image file", filename
        print >>self, "\n\n.. image:: %s\n\n" % filename

    def cmd_cvCppCross(self, c):
        self.write(":ref:`%s`" % str(c.params[0]))

    def cmd_cvCPyCross(self, c):
        self.write(":ref:`%s`" % str(c.params[0]))

    def cmd_cvfunc(self, c):
        self.cmd_cvCPyFunc(c)

    def cmd_cvCPyFunc(self, c):
        self.indent = 0
        nm = self.render(c.params[0].str)
        print >>self, "\n.. index:: %s\n" % nm
        print >>self, ".. _%s:\n" % nm
        print >>self, nm
        print >>self, '-' * len(nm)
        print >>self
        self.state = 'fpreamble'
        self.description = ""
        self.tags.append("%s\t%s\t%d" % (nm, c.filename, c.lineno))

        self.function_props = {'name' : nm}

    def cmd_cvdefPy(self, c):
        s = str(c.params[0]).replace('\\_', '_')
        print >>self, ".. function:: " + s + "\n"
        # print >>self, "=", repr(c.params[0].str)
        self.indent = 0
        print >>self, self.description
        print >>self
        self.state = None
        self.function_props['defpy'] = s

        pp.ParserElement.setDefaultWhitespaceChars(" \n\t")
        ident = pp.Word(pp.alphanums + "_.+-")
        ident_or_tuple = ident | ('(' + ident + pp.ZeroOrMore(',' + ident) + ')')
        initializer = ident_or_tuple
        arg = ident + pp.Optional('=' + initializer)
        decl = ident + '(' + pp.Optional(arg + pp.ZeroOrMore(',' + arg)) + ')' + pp.Literal("->") + ident_or_tuple + pp.StringEnd()

        try:
            l = decl.parseString(s)
            if str(l[0]) != self.function_props['name']:
                self.report_error(c, 'Decl "%s" does not match function name "%s"' % (str(l[0]), self.function_props['name']))
        except pp.ParseException, pe:
            self.report_error(c, str(pe))
            print s
            print pe

    def report_error(self, c, msg):
        print >>self.errors, "%s:%d: Error %s" % (c.filename, c.lineno, msg)

    def cmd_begin(self, c):
        self.write('\n')
        s = str(c.params[0])
        self.envstack.append((s, (c.filename, c.lineno)))
        if s == 'description':
            if 'name' in self.function_props and not 'defpy' in self.function_props:
                self.report_error(c, "No cvdefPy for function %s" % self.function_props['name'])
            self.indent += 1
        elif s == 'lstlisting':
            print >>self, "\n::\n"
            self.indent += 1
        elif s in ['itemize', 'enumerate']:
            self.indent += 1
        else:
            self.default_cmd(c)

    def cmd_item(self, c):
        self.indent -= 1
        markup = {'itemize' : '*', 'enumerate' : '#.', 'description' : '*'}[self.ee()[-1]]
        if len(c.args) > 0:
            markup += " " + str(c.args[0])
        self.write("\n\n" + markup)
        self.indent += 1

    def cmd_end(self, c):
        self.write('\n')
        s = str(c.params[0])
        if self.envstack == []:
            print "Cannot pop at", (c.filename, c.lineno)
        self.envstack.pop()
        if s == 'description':
            self.indent -= 1
        elif s in ['itemize', 'enumerate']:
            self.indent -= 1
        elif s == 'lstlisting':
            print >>self
            self.indent -= 1
        else:
            self.default_cmd(c)
        
    def cmd_label(self, c):
        pass

    def cmd_cvdefC(self, c):
        pass
    def cmd_cvdefCpp(self, c):
        pass

    # Conditionals
    def cmd_cvCpp(self, c):
        pass
    def cmd_cvC(self, c):
        pass
    def cmd_cvCPy(self, c):
        self.doL(c.params[0].str, False)
    def cmd_cvPy(self, c):
        self.doL(c.params[0].str, False)

    def render(self, L):
        """ return L rendered as a string """
        save = self.f
        self.f = StringIO.StringIO()
        for x in L:
            if isinstance(x, TexCmd):
                self.docmd(x)
            else:
                self.doplain(x)
        r = self.f.getvalue()
        self.f = save
        return r

    def cmd_cvarg(self, c):
        if self.ee() == ['description']:
            print >>self, '\n:param %s: ' % self.render(c.params[0].str),
        elif self.ee() == ['description', 'description']:
            print >>self, '\n* **%s** ' % self.render(c.params[0].str),
        else:
            print self.envstack
            assert 0
        self.indent += 1
        self.doL(c.params[1].str, False)
        self.indent -= 1
        print >>self

    def cmd_genc(self, c): pass 
    def cmd_genpy(self, c): pass 
    def cmd_author(self, c): pass 
    def cmd_cite(self, c): pass
    def cmd_date(self, c): pass
    def cmd_def(self, c): pass
    def cmd_documentclass(self, c): pass
    def cmd_maketitle(self, c): pass
    def cmd_newcommand(self, c): pass
    def cmd_newline(self, c): pass
    def cmd_setcounter(self, c): pass
    def cmd_tableofcontents(self, c): pass
    def cmd_targetlang(self, c): pass
    def cmd_usepackage(self, c): pass
    def cmd_title(self, c): pass
    def cmd_par(self, c): pass

    def cmd_href(self, c):
        self.write("`%s <%s>`_" % (str(c.params[1]), str(c.params[0])))

    def cmd_url(self, c):
        self.write(str(c.params[0]))

    def cmd_emph(self, c):
        self.write("*" + self.render(c.params[0].str) + "*")

    def cmd_textit(self, c):
        self.write("*" + self.render(c.params[0].str) + "*")

    def cmd_textbf(self, c):
        self.write("**" + self.render(c.params[0].str) + "**")

    def cmd_texttt(self, c):
        self.write("``" + self.render(c.params[0].str) + "``")

    def default_cmd(self, c):
        if self.f == self.f_section:
            self.write(repr(c))

    def unrecognized_cmd(self, c):
        if self.f == self.f_section:
            self.write(repr(c))
        self.unhandled_commands.add(c.cmd)

    def doL(self, L, newlines = True):
        for x in L:
            if isinstance(x, TexCmd):
                self.docmd(x)
            else:
                if 'lstlisting' in self.ee() or not newlines:
                    self.doplain(x)
                else:
                    self.doplain(x.lstrip())
            if self.state in ['math'] or not newlines:
                self.appendspace()
            else:
                self.write('\n')

    def ee(self):
        """ Return tags of the envstack.  envstack[0] is 'document', so skip it """
        return [n for (n,_) in self.envstack[1:]]

    def close(self):

        print >>self.errors, "unrecognized commands:"
        print >>self.errors, "\n    ".join(sorted(self.unhandled_commands))
        print >>self.errors

        open('TAGS', 'w').write("\n".join(sorted(self.tags)))

        print >>self.f_index, """

Indices and tables
==================

* :ref:`genindex`
* :ref:`search`
"""

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
sr.close()
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
