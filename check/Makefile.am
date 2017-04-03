AUTOMAKE_OPTIONS = subdir-objects
TEST_EXTENSIONS = .test

# Stu interpreter input/output tests.
LOG_COMPILER = ./runner.sh

# Functional test programs.
TEST_LOG_COMPILER = ./test_prog_runner.sh
check_PROGRAMS = test_hash.test

test_hash_test_CFLAGS = -I../src
test_hash_test_LDADD = ../src/libstu/libstu.la
test_hash_test_SOURCES = test_hash.c

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
		test_hash.test
