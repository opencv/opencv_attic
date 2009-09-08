Currently the documentation can only be built in a POSIX environment,
with a shell and the "m4" tool available.

In Ubuntu or Debian you need to install the following packages:
apt-get install texlive
apt-get install texlive-latex-extra
apt-get install latex-xcolor
apt-get install texlive-fonts-extra  
easy_install sphinx

In other Linux distros you will also need to install LiveTeX and,
optionally, the Sphinx tool (http://sphinx.pocoo.org/)

In MacOSX you can use MacTex (https://www.tug.org/mactex/).

In Windows you can use MiKTeX (not tested), but you will
also need a POSIX emulatation enviroment, like MSYS or Cygwin (not tested)
