In Ubuntu or Debian you need to install the following packages:
sudo apt-get install texlive
sudo apt-get install texlive-latex-extra
sudo apt-get install latex-xcolor
sudo apt-get install texlive-fonts-extra  
easy_install sphinx   (works on ubuntu [1] for instance using sudo synaptic and installing sphinx <not the speech recognition software>)

In other Linux distros you will also need to install LiveTeX and,
optionally, the Sphinx tool (http://sphinx.pocoo.org/)

To build the latex files, just issue the command:
sh go

In MacOSX you can use MacTex (https://www.tug.org/mactex/).

In Windows you can use MiKTeX

----
[1] To install easy install on Ubuntu, try:
 sudo apt-get install python-setuptools
 or use:
First:
  wget -q http://peak.telecommunity.com/dist/ez_setup.py
Then
  sudo python ez_setup.py
Then you can:
  easy_install -U Sphinx
