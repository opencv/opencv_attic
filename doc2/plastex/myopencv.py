
from plasTeX import Base
from plasTeX.Base.LaTeX.Verbatim import verbatim

class includegraphics(Base.Command):
  args = '[size] file'
  def invoke(self, tex):
    Base.Command.invoke(self, tex)

class cvfunc(Base.Command):
  args = 'title'
  def invoke(self, tex):
    Base.Command.invoke(self, tex)

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
