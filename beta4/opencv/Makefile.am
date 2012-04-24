# The top-level input Makefile for OpenCV

# require automake 1.4
# AUTOMAKE_OPTIONS = 1.4

## Any directories that you want built and installed should go here.
SUBDIRS = cxcore cv cvaux otherlibs apps tests samples data docs

## Any directories you want a part of the distribution should be listed
## here, as well as have a Makefile generated at the end of configure.in
##
## This only works for subdirectories one level deep.
DIST_SUBDIRS = $(SUBDIRS)

# All the rest of the distributed files
EXTRA_DIST = THANKS COPYING INSTALL README NEWS ChangeLog OpenCV.spec OpenCV.spec.in opencv.pc.in opencv.pc autogen.sh _make utils

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
