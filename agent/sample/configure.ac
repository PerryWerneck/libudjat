
dnl Process this file with autoconf to produce a configure script.

dnl The minimum version of autoconf required.
AC_PREREQ(2.59)

dnl Initialise automake with the package name, version and
dnl bug-reporting address.
AC_INIT([udjat], [0.1], [perry.werneck@gmail.com])

dnl Place auxilliary scripts here.
AC_CONFIG_AUX_DIR([scripts])

dnl Compute the canonical host-system type
AC_CANONICAL_HOST

dnl Put macro definitions here (though they aren't used).
AC_CONFIG_HEADER([src/config.h])

dnl Initialise automake stuff.
AM_INIT_AUTOMAKE

dnl Check for iconv
AM_ICONV

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_SED
AC_PROG_LN_S

AC_LANG([C++])

dnl ---------------------------------------------------------------------------
dnl Version info
dnl ---------------------------------------------------------------------------

app_vrs_major=$(echo $VERSION | cut -d. -f1)
app_vrs_minor=$(echo $VERSION | cut -d. -f2)

AC_DEFINE_UNQUOTED(PACKAGE_MAJOR_VERSION, $app_vrs_major)
AC_DEFINE_UNQUOTED(PACKAGE_MINOR_VERSION, $app_vrs_minor)

AC_SUBST(PACKAGE_MAJOR_VERSION,$app_vrs_major)
AC_SUBST(PACKAGE_MINOR_VERSION,$app_vrs_minor)

dnl ---------------------------------------------------------------------------
dnl Check for OS specifics
dnl ---------------------------------------------------------------------------

CFLAGS="$CFLAGS -I$ac_pwd/src/include"

case "$host" in
	*-mingw32)
		CFLAGS="$CFLAGS -pthread -D_WIN32_WINNT=0x0600"
		LDFLAGS="$LDFLAGS -pthread -static-libgcc -static-libstdc++"
		AC_SUBST(OSNAME,windows)
		AC_SUBST(DLLEXT,.dll)
		AC_SUBST(EXEEXT,.exe)
		;;

 	s390x-*)
		CFLAGS="$CFLAGS -pthread"
		LDFLAGS="$LDFLAGS -pthread"
		AC_SUBST(OSNAME,linux)
		AC_SUBST(DLLEXT)
		AC_SUBST(EXEEXT)
		;;


 	*)
		CFLAGS="$CFLAGS -pthread"
		LDFLAGS="$LDFLAGS -pthread"
		AC_SUBST(OSNAME,linux)
		AC_SUBST(DLLEXT)
		AC_SUBST(EXEEXT)

esac

dnl ---------------------------------------------------------------------------
dnl Check for tools
dnl ---------------------------------------------------------------------------

AC_PATH_TOOL([VALGRIND], [valgrind], [no])
AC_PATH_TOOL([AR], [ar], [ar])

dnl ---------------------------------------------------------------------------
dnl Libraries
dnl ---------------------------------------------------------------------------
PKG_CHECK_EXISTS

PKG_CHECK_MODULES( [UDJAT], [udjat], AC_DEFINE(HAVE_UDJAT), AC_MSG_ERROR([libudjat not present.]) )
AC_SUBST(UDJAT_LIBS)
AC_SUBST(UDJAT_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for doxygen
dnl ---------------------------------------------------------------------------

AC_PATH_TOOL([DOXYGEN], [doxygen])

dnl Help line for graphviz (optionally used by doxygen)
AC_ARG_ENABLE(graphviz,
	AS_HELP_STRING([--disable-graphviz], [prevent doxygen from using dot]),
	[wantgraphviz=$enableval], [wantgraphviz=yes])

AC_CHECK_PROGS([GRAPHVIZDOT], [dot])
AC_MSG_CHECKING([whether doxygen should use graphviz])
if test "x$wantgraphviz" = "xyes" ; then
	if test $GRAPHVIZDOT ; then
		AC_MSG_RESULT([yes])
		usegraphviz=yes
	else
		AC_MSG_RESULT([no, graphviz missing])
		usegraphviz=no
	fi
else
	AC_MSG_RESULT([no, disabled by configure])
	usegraphviz=no
fi

AC_SUBST(HAVE_DOT,`echo $usegraphviz | tr yesno YESNO`)

dnl ---------------------------------------------------------------------------
dnl Check for libintl
dnl ---------------------------------------------------------------------------

AC_PATH_TOOL([XGETTEXT], [xgettext], [no])
AC_PATH_TOOL([MSGCAT], [msgcat], [no])
AC_PATH_TOOL([MSGINIT], [msginit], [no])
AC_PATH_TOOL([MSGMERGE], [msgmerge], [no])
AC_PATH_TOOL([MSGFMT], [msgfmt], [no])

AC_CHECK_HEADER(libintl.h, [
	AC_DEFINE(HAVE_LIBINTL, 1)
])

dnl ---------------------------------------------------------------------------
dnl Check for C++ 2011 support
dnl ---------------------------------------------------------------------------
AC_DEFUN([AX_CHECK_COMPILE_FLAG],
[AC_PREREQ(2.59)dnl for _AC_LANG_PREFIX
AS_VAR_PUSHDEF([CACHEVAR],[ax_cv_check_[]_AC_LANG_ABBREV[]flags_$4_$1])dnl
AC_CACHE_CHECK([whether _AC_LANG compiler accepts $1], CACHEVAR, [
  ax_check_save_flags=$[]_AC_LANG_PREFIX[]FLAGS
  _AC_LANG_PREFIX[]FLAGS="$[]_AC_LANG_PREFIX[]FLAGS $4 $1"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM()],
    [AS_VAR_SET(CACHEVAR,[yes])],
    [AS_VAR_SET(CACHEVAR,[no])])
  _AC_LANG_PREFIX[]FLAGS=$ax_check_save_flags])
AS_IF([test x"AS_VAR_GET(CACHEVAR)" = xyes],
  [m4_default([$2], :)],
  [m4_default([$3], :)])
AS_VAR_POPDEF([CACHEVAR])dnl
])dnl AX_CHECK_COMPILE_FLAGS

AX_CHECK_COMPILE_FLAG([-std=c++11], [CPPFLAGS="$CPPFLAGS -std=c++11"], [AC_MSG_ERROR(["C++ 11 is required"])])

dnl ---------------------------------------------------------------------------
dnl Check for ELF symbol visibility support
dnl ---------------------------------------------------------------------------
m4_define([GLIB_CHECK_COMPILE_WARNINGS],
[m4_ifvaln([$1], [AC_LANG_CONFTEST([$1])])dnl
rm -f conftest.$ac_objext
glib_ac_compile_save="$ac_compile"
ac_compile='$CC -c $CFLAGS $CPPFLAGS conftest.$ac_ext'
AS_IF([_AC_EVAL_STDERR($ac_compile) &&
         AC_TRY_COMMAND([(if test -s conftest.err; then false ; else true; fi)])],
      [$2],
      [echo "$as_me: failed program was:" >&AS_MESSAGE_LOG_FD
cat conftest.$ac_ext >&AS_MESSAGE_LOG_FD
m4_ifvaln([$3],[$3])dnl])
ac_compile="$glib_ac_compile_save"
rm -f conftest.$ac_objext conftest.err m4_ifval([$1], [conftest.$ac_ext])[]dnl
])# GLIB_CHECK_COMPILE_WARNINGS

AC_MSG_CHECKING(for GNUC visibility attribute)
GLIB_CHECK_COMPILE_WARNINGS([
void
__attribute__ ((visibility ("hidden")))
     f_hidden (void)
{
}
void
__attribute__ ((visibility ("internal")))
     f_internal (void)
{
}
void
__attribute__ ((visibility ("protected")))
     f_protected (void)
{
}
void
__attribute__ ((visibility ("default")))
     f_default (void)
{
}
int main (int argc, char **argv)
{
        f_hidden();
        f_internal();
        f_protected();
        f_default();
        return 0;
}
],g_have_gnuc_visibility=yes,g_have_gnuc_visibility=no)
AC_MSG_RESULT($g_have_gnuc_visibility)
if test x$g_have_gnuc_visibility = xyes; then
  AC_DEFINE(HAVE_GNUC_VISIBILITY, 1, [supports GCC visibility attributes])
fi

dnl ---------------------------------------------------------------------------
dnl Check for compiler visibility flag
dnl ---------------------------------------------------------------------------
AC_MSG_CHECKING(whether $CC accepts -fvisibility=hidden)
AC_CACHE_VAL(app_cv_fvisibility_ok,
[ac_save_cc="$CC"
CC="$CC -fvisibility=hidden"
AC_TRY_RUN([int main() { return 0; }],
  app_cv_fvisibility_ok=yes,
  app_cv_fvisibility_ok=no,
  app_cv_fvisibility_ok=no)
CC="$ac_save_cc"])
AC_MSG_RESULT($app_cv_fvisibility_ok)
if test $app_cv_fvisibility_ok = yes; then
  CFLAGS="$CFLAGS -fvisibility=hidden"
fi

dnl ---------------------------------------------------------------------------
dnl Check for pic
dnl ---------------------------------------------------------------------------
AC_ARG_ENABLE([pic],
	[AS_HELP_STRING([--disable-pic], [disable support for PIC libraries (required to compile on 64bits)])],
[
	app_cv_pic="$enableval"
],[

	case "$host" in
		*-mingw32)
			app_cv_pic="no"
		;;

 	s390x-*)
			app_cv_pic="yes"
		;;

	*)
		app_cv_pic="yes"
	esac
])

if test "$app_cv_pic" == "yes"; then
	CFLAGS="$CFLAGS -fPIC"
fi

dnl ---------------------------------------------------------------------------
dnl test for pugixml
dnl ---------------------------------------------------------------------------

AC_CHECK_HEADER(pugixml.hpp, AC_DEFINE(HAVE_PUGIXML))
PUGIXML_LIBS=-lpugixml

AC_SUBST(PUGIXML_LIBS)
AC_SUBST(PUGIXML_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Directory config
dnl ---------------------------------------------------------------------------

AC_ARG_WITH([build-dir], [AS_HELP_STRING([--with-build-dir], [Setup build path])], [ app_cv_buildir="$withval" ],[ app_cv_buildir="$ac_pwd" ])
AC_SUBST(BASEDIR,$app_cv_buildir)

dnl ---------------------------------------------------------------------------
dnl Configure which files to generate.
dnl ---------------------------------------------------------------------------

AC_CONFIG_FILES(Makefile)

dnl ---------------------------------------------------------------------------
dnl Output the generated config.status script.
dnl ---------------------------------------------------------------------------
AC_OUTPUT

