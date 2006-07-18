# Contributed by Mark Asbach
# 2006-05-17, asbach@ient.rwth-aachen.de

# AC_VERSION_AT_LEAST([given-version = N[.N[.N]]],[required-version = N[.N[.N]]])
#
# Compare given-version to required-version and set ac_version_at_least to
# 'yes' or 'no' as the result. Nothing is AC_SUBSTed or AC_DEFINEd.
#
# Thanks to Eric Blake, Stephan Kasal, Paul Eggert and Harald Dunkel of
# the GNU Autoconf mailing list for providing the core implementation.
#
# Author: Mark Asbach <asbach@ient.rwth-aachen.de>
#
AC_DEFUN([AC_VERSION_AT_LEAST],[
  # check if both parameters were given
  if test -n "$1"; then
    if test -n "$2"; then
      :
    else
      AC_MSG_ERROR([no required-version supplied])
    fi
  else
    AC_MSG_ERROR([no given-version supplied])
  fi
  #
  # recombine version strings
  tmp_both=`echo "$2
$1" | sed 's/\.0*/./g'`
  tmp_sorted=`
    echo "$tmp_both" | { 
      # Use POSIX sort first, falling back on traditional sort.
      sort -t. -k1,1n -k1,1 -k2,2n -k2,2 -k3,3n -k3,3 -k4,4n -k4,4 2>/dev/null || 
      sort -t. +0n -1 +0 -1 +1n -2 +1 -2 +2n -3 +2 -3 +3n -4 +3 -4 
    }
  `
  #
  # set result value
  if test "x$tmp_both" = "x$tmp_sorted"; then
    ac_version_at_least=yes
  else
    ac_version_at_least=no
  fi
])
