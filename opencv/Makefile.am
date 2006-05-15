# The top-level input Makefile for OpenCV

# tell make where aclocal finds its files in Maintainer Mode 
ACLOCAL_AMFLAGS = -I ./autotools/aclocal

## Any directories that you want built and installed should go here.
if BUILD_APPS
#  APPS_DIR = apps
  APPS_DIR =
else
  APPS_DIR =
endif
SUBDIRS = cxcore cv cvaux otherlibs interfaces $(APPS_DIR) tests samples data docs

# All the rest of the distributed files
EXTRA_DIST = THANKS COPYING INSTALL README NEWS ChangeLog \
             opencv.spec opencv.spec.in opencv.pc.in opencv.pc \
             utils autotools
# _make apps filters (temporarily removed for Mac OS X development)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = opencv.pc

# Rule to build tar-gzipped distribution package
$(PACKAGE)-$(VERSION).tar.gz: dist

dist-hook:
	rm -rf `find $(distdir) -name CVS`
	chmod u+x $(srcdir)/utils/unix2dos.py
	find $(distdir) -name *.dsw -exec $(srcdir)/utils/unix2dos.py {} \;
	find $(distdir) -name *.dsp -exec $(srcdir)/utils/unix2dos.py {} \;

# Rule to build RPM distribution package
rpm: $(PACKAGE)-$(VERSION).tar.gz
	rpm -ta $(PACKAGE)-$(VERSION).tar.gz

# Rule to force automake to rebuild the library
changed:
	@echo "This build target is no longer necessary"

# Rule to install the libraries only - prevent rebuilding apps
install-lib:
	cd src && $(MAKE) install-libLTLIBRARIES

# Run ldconfig after installing the library:
install-hook:
	-ldconfig
