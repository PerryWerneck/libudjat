dnl SPDX-License-Identifier: LGPL-3.0-or-later

dnl Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
dnl
dnl This program is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU Lesser General Public License as published
dnl by the Free Software Foundation, either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU Lesser General Public License
dnl along with this program.  If not, see <https://www.gnu.org/licenses/>.

dnl Process this file with autoconf to produce a configure script.

dnl The minimum version of autoconf required.
AC_PREREQ([2.69])

dnl Initialise automake with the package name, version and
dnl bug-reporting address.
AC_INIT([libudjat],[2.1.0],[perry.werneck@gmail.com],[libudjat],[https://github.com/PerryWerneck/libudjat.git])

dnl Place auxilliary scripts here.
AC_CONFIG_AUX_DIR([scripts])

dnl Compute the canonical host-system type
AC_CANONICAL_HOST

dnl Put macro definitions here (though they aren't used).
AC_CONFIG_HEADER([src/include/config.h])

dnl Initialise automake stuff.
AM_INIT_AUTOMAKE

dnl Check for iconv
AM_ICONV

LT_INIT
AC_CONFIG_MACRO_DIRS([m4])

dnl Set gettext version
AM_GNU_GETTEXT_VERSION([0.14])

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_SED
AC_PROG_LN_S

AC_LANG([C++])

dnl ---------------------------------------------------------------------------
dnl Version info - https://semver.org/
dnl ---------------------------------------------------------------------------

app_vrs_major=$(echo $VERSION | cut -d. -f1)
app_vrs_minor=$(echo $VERSION | cut -d. -f2)
app_cv_domain="br.eti.werneck"
app_vrs_patch=0

AC_ARG_WITH([product-name], [AS_HELP_STRING([--with-product-name], [Set product name])], [ app_cv_product="$withval" ],[ app_cv_product="udjat" ])

AC_ARG_WITH([module-debug-path], [AS_HELP_STRING([--with-module-debug-path], [Setup module-debug-path])], [ app_cv_module_debug_path="$withval" ],[ app_cv_module_debug_path="$ac_pwd/.bin/Debug/modules/" ])
AC_SUBST(MODULE_DEBUG_PATH,$app_cv_module_debug_path)

AC_DEFINE_UNQUOTED(PRODUCT_NAME, $app_cv_product,[The product name])
AC_SUBST(PRODUCT_NAME,$app_cv_product)

AC_DEFINE_UNQUOTED(PRODUCT_DOMAIN, $app_cv_domain,[The product domain])
AC_SUBST(PRODUCT_DOMAIN,$app_cv_domain)

app_cv_title="The Eye of Horus"
AC_DEFINE_UNQUOTED(PRODUCT_TITLE,"$app_cv_title",[The product title])
AC_SUBST(PRODUCT_TITLE,"$app_cv_title")

AC_DEFINE_UNQUOTED(PACKAGE_MAJOR_VERSION, $app_vrs_major, [The library major version])
AC_DEFINE_UNQUOTED(PACKAGE_MINOR_VERSION, $app_vrs_minor, [The library minor version])
AC_DEFINE_UNQUOTED(PACKAGE_PATCH_LEVEL, $app_vrs_patch, [The patch level])

AC_SUBST(PACKAGE_MAJOR_VERSION,$app_vrs_major)
AC_SUBST(PACKAGE_MINOR_VERSION,$app_vrs_minor)
AC_SUBST(PACKAGE_PATCH_LEVEL,$app_vrs_patch)
AC_SUBST(PRODUCT_VERSION,$VERSION)

AC_SUBST(PRODUCT_VERSION_MAJOR,$app_vrs_major)
AC_SUBST(PRODUCT_VERSION_MINOR,$app_vrs_minor)
AC_SUBST(PACKAGE_VERSION_MAJOR,$app_vrs_major)
AC_SUBST(PACKAGE_VERSION_MINOR,$app_vrs_minor)
AC_SUBST(PACKAGE_VERSION_MICRO,$app_vrs_patch)

AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"libudjat-$app_vrs_major.$app_vrs_minor",[The gettext package name.])
AC_SUBST(GETTEXT_PACKAGE,"libudjat-$app_vrs_major.$app_vrs_minor")

AC_ARG_WITH([minimal-build], [AS_HELP_STRING([--with-minimal-build], [Set minimal build for modules])], [ app_cv_minbuild="$withval" ],[ app_cv_minbuild=20240612 ])
AC_DEFINE_UNQUOTED(MINIMAL_MODULE_BUILD,$app_cv_minbuild,["The required build for modules"])
AC_SUBST(MINIMAL_MODULE_BUILD,$app_cv_minbuild)

core_build_id=$(date +%y%m%d%H)
AC_SUBST(CORE_BUILD_ID,$core_build_id)
AC_SUBST(PACKAGE_CORE_BUILD,$core_build_id)

dnl ---------------------------------------------------------------------------
dnl Check for OS specifics
dnl ---------------------------------------------------------------------------

CPPFLAGS="$CPPFLAGS -I$ac_pwd/src/include"
OPTIONAL_MODULES=""

case "$host" in
	*-mingw32|*-pc-msys)
		# https://learn.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt?view=msvc-170
		CPPFLAGS="$CPPFLAGS -pthread"
		LDFLAGS="$LDFLAGS -pthread -Wl,--output-def,\$(@D)/\$(PACKAGE_NAME).def"
		LIBS="$LIBS -lintl -lshlwapi -lws2_32 -lwtsapi32 -lole32 -loleaut32 -luuid -liphlpapi -lversion -lnetshell -lpathcch"		
		AC_SUBST(LIBEXT,.dll)
		AC_SUBST(EXEEXT,.exe)
		AC_SUBST(SONAME,libudjat.dll)
		AC_SUBST(OSNAME,windows)
		AC_CONFIG_FILES(sdk/windows/static.pc)
		AC_CONFIG_FILES(sdk/windows/dynamic.pc)
		AC_CONFIG_FILES(src/main/os/windows/resources.rc)
		AC_CONFIG_FILES(src/main/os/windows/app-manifest.xml)
		AC_CONFIG_FILES(src/testprogram/windows/resources.rc)
		AC_CONFIG_FILES(src/testprogram/windows/app-manifest.xml)
		;;

 	*)
		CPPFLAGS="$CPPFLAGS -pthread -ffat-lto-objects -DLIBDIR=\"\$(libdir)\" -DDATADIR=\"\$(datarootdir)\""
		LDFLAGS="$LDFLAGS -pthread"
		LIBS="$LIBS -ldl"
		AC_SUBST(LIBEXT,.so)
		AC_SUBST(EXEEXT)
		AC_SUBST(SONAME,libudjat.so.$app_vrs_major.$app_vrs_minor)
		AC_SUBST(OSNAME,linux)
esac

AC_DEFINE_UNQUOTED(LIBEXT, "$LIBEXT", [Library extension])


dnl ---------------------------------------------------------------------------
dnl Check for tools
dnl ---------------------------------------------------------------------------

AC_PATH_TOOL([VALGRIND], [valgrind], [no])
AC_PATH_TOOL([AR], [ar], [ar])
AC_PATH_TOOL([DLLTOOL], [dlltool], [true])
AC_PATH_TOOL([STRIP], [strip], [true])
AC_PATH_TOOL([STRIP], [strip], [true])
AC_PATH_TOOL([WINDRES], [windres], [true])
AC_PATH_TOOL([DOXYGEN], [doxygen], [true])

dnl ---------------------------------------------------------------------------
dnl Check for headers
dnl ---------------------------------------------------------------------------

AC_CHECK_FUNC(localtime_r, AC_DEFINE(HAVE_LOCALTIME_R, [], [Do we have localtime_r?]) )
AC_CHECK_FUNC(localtime_s, AC_DEFINE(HAVE_LOCALTIME_S, [], [Do we have localtime_s?]) )
AC_CHECK_FUNC(strcasestr, AC_DEFINE(HAVE_STRCASESTR, [], [Do we have strcasestr?]) )

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
dnl Check for integer types & library functions
dnl ---------------------------------------------------------------------------

AC_CHECK_FUNCS(sendfile poll eventfd signal strptime)

dnl ---------------------------------------------------------------------------
dnl Check for openssl
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [LIBSSL], [libssl libcrypto], AC_DEFINE(HAVE_LIBSSL,[],[Do we have libssl?]), AC_MSG_ERROR([libssl not present.]) )

AC_SUBST(LIBSSL_LIBS)
AC_SUBST(LIBSSL_CFLAGS)
AC_DEFINE(HAVE_OPENSSL_ENGINE,[],[Do we have libssl?])

dnl ---------------------------------------------------------------------------
dnl Check for libintl
dnl ---------------------------------------------------------------------------

AC_PATH_TOOL([XGETTEXT], [xgettext], [true])
AC_PATH_TOOL([MSGCAT], [msgcat], [true])
AC_PATH_TOOL([MSGINIT], [msginit], [true])
AC_PATH_TOOL([MSGMERGE], [msgmerge], [true])
AC_PATH_TOOL([MSGFMT], [msgfmt], [true])

AC_CHECK_HEADER(libintl.h, [AC_DEFINE(HAVE_LIBINTL, 1, [Libintl is available])], [AC_MSG_NOTICE([Can't find required file libintl.h])])

dnl ---------------------------------------------------------------------------
dnl Check for C++ 2011 support
dnl ---------------------------------------------------------------------------
AC_DEFUN([AX_CHECK_COMPILE_FLAG],
[AC_PREREQ([2.69])dnl for _AC_LANG_PREFIX
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

AX_CHECK_COMPILE_FLAG([-std=c++1z], [CPPFLAGS="$CPPFLAGS -std=c++1z"], [app_cv_c17=no])

if test "$app_cv_c17" == "no"; then
	AX_CHECK_COMPILE_FLAG([-std=c++11], [CPPFLAGS="$CPPFLAGS -std=c++11 -DLIBUDJAT_LEGACY=1"], [app_cv_c11=no])
fi

dnl ---------------------------------------------------------------------------
dnl Set the ELF symbol visibility support
dnl ---------------------------------------------------------------------------
AC_DEFINE(HAVE_GNUC_VISIBILITY, 1, [supports GCC visibility attributes])

dnl ---------------------------------------------------------------------------
dnl Check for compiler visibility flag
dnl ---------------------------------------------------------------------------
AC_MSG_CHECKING(whether $CC accepts -fvisibility=hidden)
AC_CACHE_VAL(app_cv_fvisibility_ok,
[ac_save_cc="$CC"
CC="$CC -fvisibility=hidden"
AC_RUN_IFELSE([AC_LANG_SOURCE([[int main() { return 0; }]])],[app_cv_fvisibility_ok=yes],[app_cv_fvisibility_ok=no],[app_cv_fvisibility_ok=no])
CC="$ac_save_cc"])
AC_MSG_RESULT($app_cv_fvisibility_ok)
if test $app_cv_fvisibility_ok = yes; then
  CPPFLAGS="$CPPFLAGS -fvisibility=hidden"
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
		*-mingw32|*-pc-msys)
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
	CPPFLAGS="$CPPFLAGS -fPIC"
fi

dnl ---------------------------------------------------------------------------
dnl Libraries
dnl ---------------------------------------------------------------------------
PKG_CHECK_EXISTS

dnl ---------------------------------------------------------------------------
dnl Check for uriparser
dnl ---------------------------------------------------------------------------
PKG_CHECK_MODULES( [URIPARSER], [liburiparser], AC_DEFINE(HAVE_URIPARSER,[],[Do we have uriparser?]), AC_MSG_NOTICE([liburiparser not present.]) )

AC_SUBST(URIPARSER_LIBS)
AC_SUBST(URIPARSER_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl systemd integration
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [SYSTEMD], [libsystemd], AC_DEFINE(HAVE_SYSTEMD,[],[Do we have systemd?]), AC_MSG_NOTICE([libsystemd not present.]) )
AC_SUBST(SYSTEMD_CFLAGS)
AC_SUBST(SYSTEMD_LIBS)

dnl ---------------------------------------------------------------------------
dnl test for pugixml
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES(PUGIXML, pugixml, AC_DEFINE(HAVE_PUGIXML,[],[Do we have PUGIXML?]),AC_MSG_ERROR(pugixml is required))

AC_SUBST(PUGIXML_LIBS)
AC_SUBST(PUGIXML_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl test for configuration backend
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [ECONF], [libeconf], [app_cv_econf="yes"], [app_cv_econf="no"] )
AC_CHECK_HEADER(iniparser.h, [app_cv_iniparser="yes"], [app_cv_iniparser="no"])

if test "$app_cv_econf" == "yes"; then

	AC_MSG_NOTICE([ Using econf as configuration backend ])
	AC_DEFINE(HAVE_ECONF,[],[Do we have libeconf?])
	AC_SUBST(ECONF_LIBS)
	AC_SUBST(ECONF_CFLAGS)

elif test "$app_cv_iniparser" == "yes"; then

	AC_MSG_NOTICE([ Using iniparser as configuration backend ])
	AC_DEFINE(HAVE_INIPARSER,[],[Do we have libiniparser?])
	AC_SUBST(ECONF_LIBS,-liniparser)

else 

	AC_MSG_NOTICE([ No configuration backend, using only default config ])

fi

dnl ---------------------------------------------------------------------------
dnl test for vmdetect
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES(VMDETECT, vmdetect, AC_DEFINE(HAVE_VMDETECT,[],[Do we have libvmdetect?]),AC_MSG_NOTICE(No vmdetect))

AC_SUBST(VMDETECT_LIBS)
AC_SUBST(VMDETECT_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Output config
dnl ---------------------------------------------------------------------------

AC_SUBST(OPTIONAL_MODULES)
AC_SUBST(BASEDIR,$ac_pwd)

dnl ---------------------------------------------------------------------------
dnl Configure which files to generate.
dnl ---------------------------------------------------------------------------

dnl AC_CONFIG_SUBDIRS([sample])
dnl https://www.gnu.org/software/autoconf-archive/ax_subdirs_configure.html
dnl AX_SUBDIRS_CONFIGURE( [subdirs], [mandatory arguments], [possibly merged arguments], [replacement arguments], [forbidden arguments])
dnl AX_SUBDIRS_CONFIGURE([sample],[UDJAT_CFLAGS=-I$ac_pwd/src/include],[UDJAT_LIBS=-L$ac_pwd/.bin/Debug/libudjat.so.$app_vrs_major.$app_vrs_minor],[--with-build-dir=$ac_pwd])

dnl ac_configure_args="UDJAT_CFLAGS=-I$ac_pwd/src/include UDJAT_LIBS=-L$ac_pwd/.bin/Debug/libudjat.so.$app_vrs_major.$app_vrs_minor --with-build-dir=$ac_pwd"

dnl AC_CONFIG_SUBDIRS([sample])

AC_CONFIG_FILES(src/include/udjat/version.h)
AC_CONFIG_FILES(doc/doxyfile)
AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(dynamic.pc)
AC_CONFIG_FILES(static.pc)
AC_CONFIG_FILES(rpm/macros)

dnl ---------------------------------------------------------------------------
dnl Output the generated config.status script.
dnl ---------------------------------------------------------------------------

AC_OUTPUT

