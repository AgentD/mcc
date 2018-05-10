ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -D_GNU_SOURCE -I$(top_srcdir)/include
AM_CFLAGS = -std=c99 -pedantic -Wall -Wextra
AM_YFLAGS = -d

lib_LTLIBRARIES =
bin_PROGRAMS =
check_PROGRAMS =
BUILT_SOURCES =
EXTRA_DIST =
TESTS =

EXTRA_DIST += scanner.h
BUILT_SOURCES += src/parser.h

libmcc_la_SOURCES = include/str_tab.h src/str_tab.c include/ast.h src/ast.c
libmcc_la_SOURCES += src/token.h src/expr.c src/scanner.l src/parser.y
libmcc_la_SOURCES += src/stmt.c src/decl.c src/function.c src/semantic.c
libmcc_la_SOURCES += src/typecheck.c
libmcc_la_SOURCES += include/mcc.h include/literal.h include/expr.h
libmcc_la_SOURCES += include/statement.h include/decl.h include/symtab.h
libmcc_la_LFLAGS = $(AM_LFLAGS) --header-file=scanner.h

mcc2dot_SOURCES = tools/mcc2dot.c
mcc2dot_LDADD = libmcc.la

mcc_check_SOURCES = tools/mcc_check.c
mcc_check_LDADD = libmcc.la

bin_PROGRAMS += mcc2dot mcc_check
lib_LTLIBRARIES += libmcc.la

include test/Makemodule.am
