dnl
dnl AX_CHECK_GEANY([min-geany-version], [action-if-gtk3-build], [action-if-gtk2-build])
dnl
AC_DEFUN([AX_CHECK_GEANY],
[
	AC_REQUIRE([AC_PROG_GREP])
	PKG_CHECK_MODULES([GEANY], [geany >= $1])
	AC_MSG_CHECKING([whether Geany is built against GTK+ 3])
	geany_cv_gtk3_ok=$(${PKG_CONFIG} --print-requires geany | ${GREP} ^gtk+-3\.0)
	AS_IF([test "x$geany_cv_gtk3_ok" != "x"],
		[AC_MSG_RESULT([yes]); $2],
		[AC_MSG_RESULT([no]); $3])
])
