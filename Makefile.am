# The top-level input Makefile for OpenCV

# tell make where aclocal finds its files in Maintainer Mode 
ACLOCAL_AMFLAGS = -I ./autotools/aclocal

## Any directories that you want built and installed should go here.
if BUILD_APPS
  APPS_DIR = apps
else
  APPS_DIR =
endif
SUBDIRS = cxcore cv cvaux ml otherlibs utils interfaces $(APPS_DIR) tests samples data docs

# All the rest of the distributed files
#EXTRA_DIST = THANKS COPYING INSTALL README NEWS ChangeLog \
EXTRA_DIST = THANKS TODO \
             opencv.spec opencv.spec.in opencv.pc.in opencv.pc \
             _make apps filters

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = opencv.pc

# The directory where the include files will be installed
libcvincludedir = $(includedir)/opencv

# Which header files to install
libcvinclude_HEADERS =

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


FRAMEWORK_NAME = OpenCV
FRAMEWORK_VERSION = A
FRAMEWORK_CURRENT_VERSION = 2
FRAMEWORK_COMPAT_VERSION = 2

FRAMEWORK_INSTALL_NAME = @executable_path/../Frameworks/$(FRAMEWORK_NAME).framework/Versions/$(FRAMEWORK_VERSION)/$(FRAMEWORK_NAME)
FRAMEWORK_XHEADERS = ../include/tp_c.h ../include/defs.h
FRAMEWORK_OFILES = .libs/lib$(FRAMEWORK_NAME).a
FRAMEWORK_XLIBRARIES = -lpthread

FMWK_CC = g++ -dynamiclib -all_load 

framework: all clean-framework
	echo "Setting up framework directories..."
	mkdir -p $(FRAMEWORK_NAME).framework
	mkdir -p $(FRAMEWORK_NAME).framework/Versions
	mkdir -p $(FRAMEWORK_NAME).framework/Versions/$(FRAMEWORK_VERSION)
	mkdir -p $(FRAMEWORK_NAME).framework/Versions/$(FRAMEWORK_VERSION)/Resources
	mkdir -p $(FRAMEWORK_NAME).framework/Versions/$(FRAMEWORK_VERSION)/Headers
	echo "Creating framework symlinks..."
	ln -s $(FRAMEWORK_VERSION) $(FRAMEWORK_NAME).framework/Versions/Current
	ln -s Versions/Current/Headers $(FRAMEWORK_NAME).framework/Headers
	ln -s Versions/Current/Resources $(FRAMEWORK_NAME).framework/Resources
	ln -s Versions/Current/$(FRAMEWORK_NAME) $(FRAMEWORK_NAME).framework/$(FRAMEWORK_NAME)
	echo "Building framework data..."
	$(FMWK_CC) -compatibility_version $(FRAMEWORK_COMPAT_VERSION) \
		-current_version $(FRAMEWORK_CURRENT_VERSION) \
		-install_name $(FRAMEWORK_INSTALL_NAME) \
		-o $(FRAMEWORK_NAME).framework/Versions/Current/$(FRAMEWORK_NAME) \
		$(FRAMEWORK_OFILES) $(FRAMEWORK_XLIBRARIES)
	cp $(FRAMEWORK_XHEADERS) $(FRAMEWORK_NAME).framework/Headers/
	cp Info.plist $(FRAMEWORK_NAME).framework/Resources/
	cp version.plist $(FRAMEWORK_NAME).framework/Resources/

clean-framework:
	rm -rf $(FRAMEWORK_NAME).framework

