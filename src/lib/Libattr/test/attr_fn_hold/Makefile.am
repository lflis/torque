PROG_ROOT = ../..

AM_CFLAGS = -g -DTEST_FUNCTION -I${PROG_ROOT}/ -I${PROG_ROOT}/${PBS_MACH} --coverage

lib_LTLIBRARIES = libattr_fn_hold.la

AM_LDFLAGS = @CHECK_LIBS@ ${lib_LTLIBRARIES}

check_PROGRAMS = test_attr_fn_hold

libattr_fn_hold_la_SOURCES = scaffolding.c ${PROG_ROOT}/attr_fn_hold.c
libattr_fn_hold_la_LDFLAGS = @CHECK_LIBS@ -shared -lgcov

test_attr_fn_hold_SOURCES = test_attr_fn_hold.c

check_SCRIPTS = coverage_run.sh

TESTS = ${check_PROGRAMS} coverage_run.sh

coverage_run.sh:
	echo 'cp -p .libs/attr_fn_hold.gc* . >/dev/null 2>&1' > $@
	echo 'RESULTS=($$(gcov attr_fn_hold.gcda))' >> $@
	echo 'PARSED_RESULT="TOTALCOV -- $${RESULTS[1]}: Lines($${RESULTS[5]})- $${RESULTS[3]}"' >> $@
	echo 'echo -e "\033[40m\033[1;33m$$PARSED_RESULT\033[0m"' >> $@
	chmod +x $@

CLEANFILES = coverage_run.sh *.gcno *.gcda *.gcov core *.lo
