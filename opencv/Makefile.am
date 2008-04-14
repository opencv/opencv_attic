# The top-level input Makefile for OpenCV

# use the default c++ flags
AM_CXXFLAGS=@DEF_CXXFLAGS@

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
             _make apps filters make_frameworks.sh

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
install-exec-hook:
	-ldconfig

# rule that gets added to the clean target by Automake
clean-local: clean-framework


# FRAMEWORK ###################################################################
#
# The following section is dedicated to Darwin / Mac OS X framework builds.
# It currently creates a private framework (see Apple documentation) that
# cannot be placed to the /Library/Framework locations but is intended to get
# copied into application bundles.
# 
# You should usually not invoke this target directly but use the shell script
# 'make_frameworks.sh' instead that will create the most sensible
# default installation (universal build).
# 
# See: http://developer.apple.com/documentation/MacOSX/Conceptual/BPFrameworks/
# 

FRAMEWORK_NAME = OpenCV
FRAMEWORK_VERSION = A
FRAMEWORK_CURRENT_VERSION = 1
FRAMEWORK_COMPAT_VERSION = 1

# install path to be set in calling script
# FRAMEWORK_INSTALL_PATH = executable_path/../Frameworks

# architecture to be set in calling script
# FRAMEWORK_ARCH = i386

FRAMEWORK_INSTALL_NAME = $(FRAMEWORK_INSTALL_PATH)/$(FRAMEWORK_NAME).framework/Versions/$(FRAMEWORK_VERSION)/$(FRAMEWORK_NAME)

FRAMEWORK_XHEADERS = \
  $(top_srcdir)/autotools/darwin/OpenCV.h     \
  $(top_srcdir)/cxcore/include/cxcore.h       \
  $(top_srcdir)/cxcore/include/cxerror.h      \
  $(top_srcdir)/cxcore/include/cxtypes.h      \
  $(top_srcdir)/cxcore/include/cxmisc.h       \
  $(top_srcdir)/cxcore/include/cxcore.hpp     \
  $(top_srcdir)/cxcore/include/cvver.h        \
  $(top_srcdir)/cv/include/cv.h               \
  $(top_srcdir)/cv/include/cvcompat.h         \
  $(top_srcdir)/cv/include/cvtypes.h          \
  $(top_srcdir)/cv/include/cv.hpp             \
  $(top_srcdir)/cvaux/include/cvaux.h         \
  $(top_srcdir)/cvaux/include/cvmat.hpp       \
  $(top_srcdir)/cvaux/include/cvaux.hpp       \
  $(top_srcdir)/cvaux/include/cvvidsurv.hpp   \
  $(top_srcdir)/ml/include/ml.h               \
  $(top_srcdir)/otherlibs/highgui/highgui.h

FRAMEWORK_OFILES = \
  $(top_builddir)/cxcore/src/.libs/lib_cxcore.a  \
  $(top_builddir)/cv/src/.libs/lib_cv.a          \
  $(top_builddir)/cvaux/src/.libs/lib_cvaux.a    \
  $(top_builddir)/ml/src/.libs/lib_ml.a          \
  $(top_builddir)/otherlibs/highgui/.libs/lib_highgui.a

FRAMEWORK_XLIBRARIES = -framework Carbon -framework QuickTime -framework QuartzCore

FRAMEWORK_DOCUMENTATION = \
  $(top_srcdir)/docs/license.txt \
  $(top_srcdir)/docs/index.htm   \
  $(top_srcdir)/docs/faq.htm     \
  $(top_srcdir)/docs/ref

FMWK_CC = g++ -dynamiclib -all_load -Wl,-single_module -Wl,-Y,100

framework: all clean-framework
	@echo "Setting up framework directories..."
	mkdir -p $(FRAMEWORK_NAME).framework
	mkdir -p $(FRAMEWORK_NAME).framework/Versions
	mkdir -p $(FRAMEWORK_NAME).framework/Versions/$(FRAMEWORK_VERSION)
	mkdir -p $(FRAMEWORK_NAME).framework/Versions/$(FRAMEWORK_VERSION)/Resources
	mkdir -p $(FRAMEWORK_NAME).framework/Versions/$(FRAMEWORK_VERSION)/Headers
	mkdir -p $(FRAMEWORK_NAME).framework/Versions/$(FRAMEWORK_VERSION)/Documentation
	@echo "Creating framework symlinks..."
	ln -s $(FRAMEWORK_VERSION)               $(FRAMEWORK_NAME).framework/Versions/Current
	ln -s Versions/Current/Headers           $(FRAMEWORK_NAME).framework/Headers
	ln -s Versions/Current/Resources         $(FRAMEWORK_NAME).framework/Resources
	ln -s Versions/Current/Documentation     $(FRAMEWORK_NAME).framework/Documentation
	ln -s Versions/Current/$(FRAMEWORK_NAME) $(FRAMEWORK_NAME).framework/$(FRAMEWORK_NAME)
	@echo "Building framework data..."
	$(FMWK_CC) \
		-arch $(FRAMEWORK_ARCH) \
		-compatibility_version $(FRAMEWORK_COMPAT_VERSION) \
		-current_version $(FRAMEWORK_CURRENT_VERSION) \
		-install_name $(FRAMEWORK_INSTALL_NAME) \
		-o $(FRAMEWORK_NAME).framework/Versions/Current/$(FRAMEWORK_NAME) \
		$(FRAMEWORK_OFILES) $(FRAMEWORK_XLIBRARIES)
	strip -x $(FRAMEWORK_NAME).framework/Versions/Current/$(FRAMEWORK_NAME)
	cp $(FRAMEWORK_XHEADERS) $(FRAMEWORK_NAME).framework/Headers/
	cp -r $(FRAMEWORK_DOCUMENTATION) $(FRAMEWORK_NAME).framework/Documentation/
	cp $(top_srcdir)/autotools/darwin/Info.plist $(FRAMEWORK_NAME).framework/Resources/

clean-framework:
	rm -rf $(FRAMEWORK_NAME).framework
