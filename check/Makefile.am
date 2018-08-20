AUTOMAKE_OPTIONS = subdir-objects
TEST_EXTENSIONS = .test .input

# Stu interpreter input/output tests.
INPUT_LOG_COMPILER = ./runner.sh

# Functional test programs.
TEST_LOG_COMPILER = ./test_prog_runner.sh
check_PROGRAMS = test_hash.test test_multi_stu.test test_valid_form.test

test_hash_test_CFLAGS = -I$(top_srcdir)/src
test_hash_test_LDADD = $(top_builddir)/src/libstu/libstu.la
test_hash_test_SOURCES = test_hash.c

test_multi_stu_test_CFLAGS = -I$(top_srcdir)/src
test_multi_stu_test_LDADD = $(top_builddir)/src/libstu/libstu.la
test_multi_stu_test_SOURCES = test_multi_stu.c

test_valid_form_test_CFLAGS = -I$(top_srcdir)/src
test_valid_form_test_LDADD = $(top_builddir)/src/libstu/libstu.la
test_valid_form_test_SOURCES = test_valid_form.c

# All tests specified below.
TESTS = 001_curried.input \
		002_numbers.input \
		003_types.input \
		004_quote.input \
		005_var_args.input \
		006_var_args.input \
		007_comparison.input \
		008_if.input \
		009_not.input \
		010_pred.input \
		011_map.input \
		012_partial_eval.input \
		013_nil_test.input \
		014_test_accessors.input \
		015_scoping.input \
		016_real.input \
		017_newton_root_2.input \
		018_rational.input \
		019_read.input \
		020_lambda_no_args.input \
		021_macroexpand.input \
		022_native_closure.input \
		023_type_of.input \
		024_arity_checks.input \
		025_re.input \
		026_try_and_throw.input \
		027_try_no_throw.input \
		028_nested_try.input \
                029_def_in_lambda.input \
		test_hash.test \
		test_multi_stu.test \
		test_valid_form.test
