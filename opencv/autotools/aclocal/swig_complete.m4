# Contributed by Sebastian Huber

# SWIG_PROG([required-version = N[.N[.N]]])
#
# Checks for the SWIG program.  If found you can (and should) call
# SWIG via $(SWIG).  You can use the optional first argument to check
# if the version of the available SWIG is greater than or equal to the
# value of the argument.  It should have the format: N[.N[.N]] (N is a
# number between 0 and 999.  Only the first N is mandatory.)
AC_DEFUN([SWIG_PROG],[
	AC_DIAGNOSE([obsolete],[Please replace calls to SWIG_PROG by AC_PATH_SWIG])
	AC_PATH_SWIG($1)
])

AC_DEFUN([AC_PATH_SWIG],[
	AC_PATH_PROG([SWIG],[swig])
	if test -z "$SWIG" ; then
		AC_MSG_WARN([cannot find 'swig' program, you may have a look at http://www.swig.org])
	else
		AC_MSG_CHECKING([for SWIG version])
		[SWIG_VERSION=`$SWIG -version 2>&1 | grep 'SWIG Version' | sed 's/.*\([0-9]\+\.[0-9]\+\.[0-9]\+\).*/\1/g'`]
		AC_MSG_RESULT([$SWIG_VERSION])
		if test -n "$SWIG_VERSION"; then
			if test -n "$1" ; then
				# Calculate the required version number
				[swig_tmp=( `echo $1 | sed 's/[^0-9]\+/ /g'` )]
				[swig_required_version=$(( 1000000 * ${swig_tmp[0]:-0} + 1000 * ${swig_tmp[1]:-0} + ${swig_tmp[2]:-0} ))]

				# Calculate the available version number
				[swig_tmp=( `echo $SWIG_VERSION | sed 's/[^0-9]\+/ /g'` )]
				[swig_tmp=$(( 1000000 * ${swig_tmp[0]:-0} + 1000 * ${swig_tmp[1]:-0} + ${swig_tmp[2]:-0} ))]

				if test $swig_required_version -gt $swig_tmp ; then
					AC_MSG_WARN([SWIG version $1 is required, you have $SWIG_VERSION])
				fi
			fi
		else
			AC_MSG_WARN([cannot determine SWIG version])
		fi

		SWIG_RUNTIME_LIBS_DIR="${SWIG%/bin*}/lib"
		AC_MSG_NOTICE([SWIG runtime library directory is '$SWIG_RUNTIME_LIBS_DIR'])
	fi
	AC_SUBST([SWIG_VERSION])
	AC_SUBST([SWIG_RUNTIME_LIBS_DIR])
])

# SWIG_ENABLE_CXX()
#
# Enable SWIG C++ support.  This effects all invocations of $(SWIG).
AC_DEFUN([SWIG_ENABLE_CXX],[
	AC_REQUIRE([AC_PATH_SWIG])
	AC_REQUIRE([AC_PROG_CXX])
	if test -z "$SWIG" ; then
		AC_MSG_ERROR([swig not found])
	fi
	SWIG="$SWIG -c++"
])

# SWIG_MULTI_MODULE_SUPPORT()
#
# Enable support for multiple modules.  This effects all invocations
# of $(SWIG).  You have to link all generated modules against the
# appropriate SWIG runtime library.  If you want to build Python
# modules for example, use the SWIG_PYTHON() macro and link the
# modules against $(SWIG_PYTHON_LIBS).
AC_DEFUN([SWIG_MULTI_MODULE_SUPPORT],[
	AC_REQUIRE([AC_PATH_SWIG])

	# Calculate the available version number
	AC_MSG_CHECKING([for swig multi module support])
	if test -z "$SWIG" ; then
		AC_MSG_ERROR([swig not found])
	fi

	[swig_tmp=( `echo $SWIG_VERSION | sed 's/[^0-9]\+/ /g'` )]
	[swig_tmp=$(( 1000000 * ${swig_tmp[0]:-0} + 1000 * ${swig_tmp[1]:-0} + ${swig_tmp[2]:-0} ))]
	if test -n "$SWIG_VERSION"; then
		if test $swig_tmp -ge "1003024" ; then
			AC_MSG_RESULT([always working (SWIG >= 1.3.24)])
		else
			if test $swig_tmp -ge "1003020" ; then
				SWIG="$SWIG -noruntime"
				AC_MSG_RESULT([interim syntax (SWIG >= 1.3.20 < 1.3.24)])
			else
				SWIG="$SWIG -c"
				AC_MSG_RESULT([old syntax (SWIG < 1.3.20)])
			fi
		fi
	else
		AC_MSG_WARN([SWIG version number unknown - couldn't set flags for multi module support])
	fi
])

# SWIG_PYTHON([use-shadow-classes = {no, yes}])
#
# Checks for Python and provides the $(SWIG_PYTHON_CPPFLAGS),
# $(SWIG_PYTHON_LIBS) and $(SWIG_PYTHON_OPT) output variables.
# $(SWIG_PYTHON_OPT) contains all necessary SWIG options to generate
# code for Python.  Shadow classes are enabled unless the value of the
# optional first argument is exactly 'no'.  If you need multi module
# support (provided by the SWIG_MULTI_MODULE_SUPPORT() macro) use
# $(SWIG_PYTHON_LIBS) to link against the appropriate library.  It
# contains the SWIG Python runtime library that is needed by the type
# check system for example.
AC_DEFUN([SWIG_PYTHON],[
	AC_REQUIRE([AC_PATH_SWIG])
	AC_REQUIRE([AM_PATH_PYTHON])
	test "x$1" != "xno" || swig_shadow=" -noproxy"

	[swig_tmp=( `echo $SWIG_VERSION | sed 's/[^0-9]\+/ /g'` )]
	[swig_tmp=$(( 1000000 * ${swig_tmp[0]:-0} + 1000 * ${swig_tmp[1]:-0} + ${swig_tmp[2]:-0} ))]
	if test -n "$SWIG_VERSION"; then
		if test $swig_tmp -ge "1003024" ; then
			SWIG_PYTHON_LIBS=""
		else
			SWIG_PYTHON_LIBS="$SWIG_RUNTIME_LIBS_DIR -lswigpy"
		fi
	else
		AC_MSG_WARN([SWIG version number unknown - couldn't set python libs])
	fi

	AC_SUBST([SWIG_PYTHON_OPT],  "-python$swig_shadow")
	AC_SUBST([SWIG_PYTHON_LIBS])
])

# PYTHON_DEVEL()
#
# Checks for Python and tries to get the include path to 'Python.h'.
# It provides the $(PYTHON_CPPFLAGS) and $(PYTHON_LDFLAGS) output variable.
AC_DEFUN([PYTHON_DEVEL],[
	AC_REQUIRE([AM_PATH_PYTHON])

	# Check for Python include path
	AC_MSG_CHECKING([for Python include path])
	python_path=${PYTHON%/bin*}
	for i in "$python_path/include/python$PYTHON_VERSION/" "$python_path/include/python/" "$python_path/" ; do
		python_path=`find $i -type f -name Python.h -print`
		if test -n "$python_path" ; then
			break
		fi
	done
	for i in $python_path ; do
		python_path=${python_path%/Python.h}
		break
	done
	AC_MSG_RESULT([$python_path])
	if test -z "$python_path" ; then
		AC_MSG_ERROR([cannot find Python include path])
	fi
	AC_SUBST([PYTHON_CPPFLAGS],[-I$python_path])

	# Check for Python library path
	AC_MSG_CHECKING([for Python library path])
	python_path=${PYTHON%/bin*}
	for i in "$python_path/lib/python$PYTHON_VERSION/config/" "$python_path/lib/python$PYTHON_VERSION/" "$python_path/lib/python/config/" "$python_path/lib/python/" "$python_path/" ; do
		python_path=`find $i -type f -name libpython$PYTHON_VERSION.* -print`
		if test -n "$python_path" ; then
			break
		fi
	done
	for i in $python_path ; do
		python_path=${python_path%/libpython*}
		break
	done
	AC_MSG_RESULT([$python_path])
	if test -z "$python_path" ; then
		AC_MSG_ERROR([cannot find Python library path])
	fi
	AC_SUBST([PYTHON_LDFLAGS],["-L$python_path -lpython$PYTHON_VERSION"])
])
