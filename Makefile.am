# The top-level input Makefile for OpenCV

# tell make where aclocal finds its files in Maintainer Mode 
ACLOCAL_AMFLAGS = -I ./autotools/aclocal

## Any directories that you want built and installed should go here.
if BUILD_APPS
  APPS_DIR = apps
else
  APPS_DIR =
endif
SUBDIRS = cxcore cv cvaux otherlibs interfaces $(APPS_DIR) tests samples data docs

## Any directories you want a part of the distribution should be listed
## here, as well as have a Makefile generated at the end of configure.in
##
## This only works for subdirectories one level deep.
DIST_SUBDIRS = $(SUBDIRS) apps

# All the rest of the distributed files
EXTRA_DIST = THANKS COPYING INSTALL README NEWS ChangeLog opencv.spec opencv.spec.in opencv.pc.in opencv.pc _make utils autotools

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = opencv.pc

# Rule to build tar-gzipped distribution package
$(PACKAGE)-$(VERSION).tar.gz: dist

dist-hook:
	rm -rf `find $(distdir) -name CVS`
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
