#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([main/main.cpp])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE(server,1.0)
# Checks for programs.
AC_PROG_CXX
#AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries.
# FIXME: Replace `main' with a function in `-ltcpserver':
AC_CHECK_LIB([tcpserver], [main])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h sys/socket.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([socket])

AC_CONFIG_FILES([Makefile
                 main/Makefile
                 testClient/Makefile])
AC_OUTPUT
