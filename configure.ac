#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.60)

# Set program name, version, bug-address and source directory
m4_define([JV_VERSION],[1.8.4])
AC_INIT(jv,JV_VERSION,tris@projectaten.net)
AC_CONFIG_SRCDIR([src/])

# Check system type and set defines
# Specify a single file in the source tree so we can check that --srcdir points to the right place
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADER(config.h)
AC_CANONICAL_TARGET
MAC_LIBS=""
if test "$target_vendor" = "apple"; then
  AC_DEFINE([IS_MAC], [], [Whether operating system is an Apple])
fi
AC_SUBST(MAC_LIBS)

AM_INIT_AUTOMAKE()

# Checks for programs.
AC_PROG_CXX
AC_PROG_F77
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_YACC

#
# Check if compilers work
#
# -- g++
AC_MSG_CHECKING(if $CXX works)
cxx_compiler_works=no
AC_LANG(C++)
AC_COMPILE_IFELSE([AC_LANG_SOURCE([int main() { return 0; }])], [cxx_compiler_works=yes], [CXX=""] )
AC_MSG_RESULT($cxx_compiler_works)
if test "$cxx_compiler_works" = "no"; then
  AC_MSG_ERROR([C++ compiler doesn't appear to work.])
fi
# -- Fortran
AC_MSG_CHECKING(if $F77 works)
f77_compiler_works=no
AC_LANG(Fortran 77)
AC_COMPILE_IFELSE(AC_LANG_PROGRAM([],[]), [f77_compiler_works=yes], [F77=""] )
AC_MSG_RESULT($f77_compiler_works)
if test "$f77_compiler_works" = "no"; then
  AC_MSG_ERROR([Fortran compiler doesn't appear to work.])
fi

#
# Additional Configuration Options
#

# Lite Version Toggle
AC_ARG_ENABLE([lite], [  --enable-lite    Turn on lite version],
[case "${enableval}" in
  yes) lite=true ;;
  no)  lite=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-lite]) ;;
esac],[lite=false])
AM_CONDITIONAL([LITE], [test x$lite = xtrue])
LITE_CFLAGS=""
AM_COND_IF([LITE], LITE_CFLAGS="-DLITE")

# QT Tools
AC_ARG_WITH([qtmoc],[AC_HELP_STRING([--with-qtmoc=<executable>],
    [Specify Qt5 Meta-Object Compiler tool (moc) [default="moc-qt5"]])], ,
    [with_qtmoc="moc-qt5"])
AC_ARG_WITH([qtuic],[AC_HELP_STRING([--with-qtuic=<executable>],
    [Specify Qt5 User Interface Compiler tool (uic) [default="uic-qt5"]])], ,
    [with_qtuic="uic-qt5"])
AC_ARG_WITH([qtrcc],[AC_HELP_STRING([--with-qtrcc=<executable],
    [Specify Qt5 Resource Compiler tool (rcc) [default="rcc-qt5"]])], ,
    [with_qtrcc="rcc-qt5"])
QTMOC="$with_qtmoc"
QTRCC="$with_qtrcc"
QTUIC="$with_qtuic"

# Build Directory (used when packaging)
BUILDDIR=""
AC_ARG_WITH([build-dir],[AC_HELP_STRING([--with-build-dir=<dir>],
    [Specify location of build environment when packaging [default=""]])], ,
    [with_build_dir=""])
BUILDDIR="$with_build_dir"

# HDF library to link against
HDFLIB="hdf5"
AC_ARG_WITH([hdf-lib],[AC_HELP_STRING([--with-hdf-lib=<lib>],
    [Specify name of HDF5 library to link against [default="hdf5"]])], ,
    [with_hdf_lib="hdf5"])
HDFLIB="$with_hdf_lib"

# Install Directory (used when packaging - should be '$PREFIX - $BUILDDIR')
INSTALLDIR="/usr/local"
AC_ARG_WITH([install-dir],[AC_HELP_STRING([--with-install-dir=<dir>],
    [Specify 'clean' instal dir prefix, i.e. without any BUILDDIR [default="/usr/local"]])], ,
    [with_install_dir="/usr/local"])
# If --prefix has been give, use this value by default
if test "$prefix" != "NONE"; then
  INSTALLDIR="$prefix"
fi

if test "$with_install_dir" != "/usr/local"; then
  INSTALLDIR="$with_install_dir"
fi

# Pre-initialise pkg-config
PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(QTGUI, Qt5Gui >= 5.0.0)
PKG_CHECK_MODULES(QTNETWORK, Qt5Network >= 5.0.0)
PKG_CHECK_MODULES(QTWIDGETS, Qt5Widgets >= 5.0.0)
PKG_CHECK_MODULES(QTSVG, Qt5Svg >= 5.0.0)
PKG_CHECK_MODULES(QTPRINT, Qt5PrintSupport >= 5.0.0)
QTGUI_LDLIBS="$QTNETWORK_LIBS $QTWIDGETS_LIBS $QTGUI_LIBS $QTSVG_LIBS $QTPRINT_LIBS -l$HDFLIB"
QTGUI_CFLAGS="$QTWIDGETS_CFLAGS $QTNETWORK_CFLAGS $QTSVG_CFLAGS $QTPRINT_CFLAGS"

JVLDLIBS="$QTGUI_LDLIBS -lgfortran"
JVCFLAGS="-fPIC $LITE_CFLAGS $QTGUI_CFLAGS"


VERSION=JV_VERSION

# Perform Makefile substitutions
AC_SUBST(JVLDLIBS)
AC_SUBST(JVCFLAGS)
AC_SUBST(QTMOC)
AC_SUBST(QTUIC)
AC_SUBST(QTRCC)
AC_SUBST(BUILDDIR)
AC_SUBST(INSTALLDIR)
AC_SUBST(VERSION)


# Checks for typedefs, structures, and compiler characteristics.
AC_LANG(C++)
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_STRUCT_TM


# Checks for library functions.
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([floor pow select sqrt strchr strrchr strstr])


# Define Makefiles in AC_OUTPUT 
AC_OUTPUT(Makefile
src/Makefile
src/get/Makefile
)
