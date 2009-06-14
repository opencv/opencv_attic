
from plasTeX import Base
from plasTeX.Base.LaTeX.Verbatim import verbatim
from plasTeX.Base.LaTeX import Sectioning
import sys

class includegraphics(Base.Command):
  args = '[size] file'
  def invoke(self, tex):
    Base.Command.invoke(self, tex)

class cvfunc(Sectioning.subsection):
  def invoke(self, tex):
    Sectioning.subsection.invoke(self, tex)

class cvstruct(Sectioning.subsection):
  def invoke(self, tex):
    Sectioning.subsection.invoke(self, tex)

class cvmacro(Sectioning.subsection):
  def invoke(self, tex):
    Sectioning.subsection.invoke(self, tex)

class cross(Base.Command):
  args = 'name'
  def invoke(self, tex):
    Base.Command.invoke(self, tex)

class label(Base.Command):
  args = 'name'
  def invoke(self, tex):
    Base.Command.invoke(self, tex)

class url(Base.Command):
  args = 'loc'
  def invoke(self, tex):
    Base.Command.invoke(self, tex)

class cvarg(Base.Command):
  args = 'item def'
  def invoke(self, tex):
    Base.Command.invoke(self, tex)

class cvexp(Base.Command):
  args = 'c cpp py'
  def invoke(self, tex):
    Base.Command.invoke(self, tex)

class lstlisting(verbatim):
  def parse(self, tex):
    verbatim.parse(self, tex)
    return self.attributes

def section_filename(title):
    """Image Processing ==> image_processing.rst"""
    lower_list = [word.lower() for word in title.split()]
    return "_".join(lower_list) + ".rst"

class chapter(Sectioning.chapter):
    @property
    def filenameoverride(self):
        if self.attributes['title'] is not None:
            filename = section_filename(str(self.attributes['title']))
            assert filename in ['cxcore.rst', 'cvreference.rst']
            return filename
        raise AttributeError, 'This chapter does not generate a new file'
        

class section(Sectioning.section):
    @property
    def filenameoverride(self):
        if self.attributes['title'] is not None:
            filename = section_filename(str(self.attributes['title']))
            print 'section:', filename
            return filename
        raise AttributeError, 'This section does not generate a new file'


