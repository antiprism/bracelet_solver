#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([bracelet_solver], [0.02], [adrian@antiprism.com])
AM_INIT_AUTOMAKE([1.9 foreign])

AC_CONFIG_SRCDIR([src/utils.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

AC_LANG([C++])
AX_CXX_COMPILE_STDCXX(11)


# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([malloc.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([strspn])

AC_CONFIG_FILES([Makefile
                 src/Makefile])

AC_OUTPUT
