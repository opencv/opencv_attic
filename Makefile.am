EXTRA_DIST = Makefile.gcc Makefile.bcc Makefile.icl Makefile.vc docs utils

SUBDIRS = cv cvaux otherlibs @ADDONS@ bin @TESTDIR@

DIST_SUBDIRS = cv cvaux otherlibs bin apps tests

AUTOMAKE_OPTIONS = foreign

# this targets create distributive with symbolic links
dist-l: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) cozf $(distdir).tar.gz $(distdir)
	-rm -rf $(distdir)
dist-all-l: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) cozf $(distdir).tar.gz $(distdir)
	-rm -rf $(distdir)
