AC_PREREQ([2.60])

AC_INIT([mcc], 0.1, [david.oberhollenzer@student.uibk.ac.at], mcc)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign subdir-objects dist-xz])
AM_SILENT_RULES([yes])
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_LEX
AC_PROG_YACC
LT_INIT()

AC_ARG_ENABLE([install-tests],
	[AS_HELP_STRING([--enable-install-tests],
			[Install unit test programs])],
	[case "${enableval}" in
	yes) AM_CONDITIONAL([INSTALL_TESTS], [true]) ;;
	no)  AM_CONDITIONAL([INSTALL_TESTS], [false]) ;;
	*) AC_MSG_ERROR([bad value ${enableval} for --enable-install-tests]) ;;
	esac],
	[AM_CONDITIONAL([INSTALL_TESTS], [false])])

AC_OUTPUT([Makefile])
