AC_INIT([Geany++], [0.1], [matt@geany.org], [geany++])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_SILENT_RULES([yes])
AM_PATH_PYTHON([3.3])
AM_PROG_AR
LT_INIT([disable-static])
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])
AX_CHECK_GEANY([1.28],
	[PKG_CHECK_MODULES([GTKMM], [gtkmm-3.0])],
	[PKG_CHECK_MODULES([GTKMM], [gtkmm-2.4])])
AC_CONFIG_HEADERS([geany++/config.h])
AC_CONFIG_FILES([Makefile geany++/Makefile])
AC_OUTPUT
