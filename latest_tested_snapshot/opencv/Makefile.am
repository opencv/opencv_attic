# The top-level input Makefile for OpenCV

# tell make where aclocal finds its files in Maintainer Mode
ACLOCAL_AMFLAGS = -I autotools/aclocal

## Any directories that you want built and installed should go here.
if BUILD_APPS
  APPS_DIR = apps
else
  APPS_DIR =
endif
SUBDIRS = 3rdparty src include utils interfaces $(APPS_DIR) tests samples data

# All the rest of the distributed files
EXTRA_DIST = \
  THANKS \
  TODO \
  opencv.spec \
  opencv.spec.in \
  opencv.pc \
  opencv.pc.in \
  \
  make_frameworks.sh \
  autotools/darwin/Info.plist \
  autotools/darwin/OpenCV.h \
  \
  CMakeLists.txt \
  opencv.pc.cmake.in \
  Package.cmake.in \
  cmake_uninstall.cmake.in \
  OpenCVConfig.cmake.in \
  OpenCVFindPkgConfig.cmake \
  OpenCVPCHSupport.cmake \
  cvconfig.h.cmake \
  \
  doc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = opencv.pc

# Rule to build tar-gzipped distribution package
$(PACKAGE)-$(VERSION).tar.gz: dist

dist-hook:
	rm -rf `find $(distdir) -name .svn`

# Rule to build RPM distribution package
rpm: $(PACKAGE)-$(VERSION).tar.gz
	rpm -ta $(PACKAGE)-$(VERSION).tar.gz

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
FRAMEWORK_VERSION = B
FRAMEWORK_CURRENT_VERSION = 0
FRAMEWORK_COMPAT_VERSION = 0

# install path to be set in calling script
# FRAMEWORK_INSTALL_PATH = executable_path/../Frameworks

# architecture to be set in calling script
# FRAMEWORK_ARCH = i386

FRAMEWORK_INSTALL_NAME = $(FRAMEWORK_INSTALL_PATH)/$(FRAMEWORK_NAME).framework/Versions/$(FRAMEWORK_VERSION)/$(FRAMEWORK_NAME)

FRAMEWORK_XHEADERS = \
  $(top_srcdir)/autotools/darwin/OpenCV.h     \
  $(top_srcdir)/include/opencv/cv.h           \
  $(top_srcdir)/include/opencv/cv.hpp         \
  $(top_srcdir)/include/opencv/cvaux.h        \
  $(top_srcdir)/include/opencv/cvaux.hpp      \
  $(top_srcdir)/include/opencv/cvcompat.h     \
  $(top_srcdir)/include/opencv/cvstreams.h    \
  $(top_srcdir)/include/opencv/cvtypes.h      \
  $(top_srcdir)/include/opencv/cvver.h        \
  $(top_srcdir)/include/opencv/cvvidsurv.hpp  \
  $(top_srcdir)/include/opencv/cvwimage.h     \
  $(top_srcdir)/include/opencv/cxcore.h       \
  $(top_srcdir)/include/opencv/cxcore.hpp     \
  $(top_srcdir)/include/opencv/cxerror.h      \
  $(top_srcdir)/include/opencv/cxflann.h      \
  $(top_srcdir)/include/opencv/cxmat.hpp      \
  $(top_srcdir)/include/opencv/cxmisc.h       \
  $(top_srcdir)/include/opencv/cxoperations.hpp \
  $(top_srcdir)/include/opencv/cxtypes.h      \
  $(top_srcdir)/include/opencv/highgui.h      \
  $(top_srcdir)/include/opencv/highgui.hpp    \
  $(top_srcdir)/include/opencv/ml.h


FRAMEWORK_OFILES = \
  $(top_builddir)/3rdparty/.libs/lib_clapack.a  \
  $(top_builddir)/3rdparty/.libs/lib_flann.a    \
  $(top_builddir)/src/.libs/lib_cxcore.a        \
  $(top_builddir)/src/.libs/lib_cv.a            \
  $(top_builddir)/src/.libs/lib_cvaux.a         \
  $(top_builddir)/src/.libs/lib_ml.a            \
  $(top_builddir)/src/.libs/lib_highgui.a

FRAMEWORK_XLIBRARIES = -lz -framework Carbon -framework QuickTime -framework QuartzCore

FRAMEWORK_DOCUMENTATION = 
#\
#  $(top_srcdir)/doc/license.txt \
#  $(top_srcdir)/doc/index.htm   \
#  $(top_srcdir)/doc/faq.htm     \
#  $(top_srcdir)/doc/ref

FMWK_CC = g++-4.2 -fopenmp -dynamiclib -all_load -Wl,-single_module -Wl,-Y,100

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
	cp $(top_srcdir)/autotools/darwin/Info.plist $(FRAMEWORK_NAME).framework/Resources/

#	cp -r $(FRAMEWORK_DOCUMENTATION) $(FRAMEWORK_NAME).framework/Documentation/

clean-framework:
	rm -rf $(FRAMEWORK_NAME).framework build_i386 build_ppc build_x86_64 build_ppc64
