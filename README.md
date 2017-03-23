### Stu; another lisp implementation

[![Build Status](https://travis-ci.org/mikey-austin/stutter.svg?branch=master)](https://travis-ci.org/mikey-austin/stutter)

Stu is a simple & minimal lisp implementation in C.

    $ make >/dev/null
    $ cat <<END |./stu -f-
    (def a 100)
    (def b 200)
    (def c (+ a (* 2 b)))
    (eval (list 'quote a b c))
    END
    (100 200 500)

Or you can use the repl by specifying no arguments:

    $ ./stu
    stu> (def make-adder (λ (a) (λ (b) (+ a b))))
    nil
    stu> (def add-two (make-adder 2))
    nil
    stu> (add-two 2)
    4
    stu> ^D
    Bye!

### Tests

The test suite uses valgrind by default. To disable valgrind, use the following:

    $ make WITH_VALGRIND= test
    cd test; /bin/bash ./runner.sh  -f ../stu
    (1/7) Testing 1_curried                [ OK ]
    (2/7) Testing 1_numbers                [ OK ]
    (3/7) Testing 1_types                  [ OK ]
    (4/7) Testing 2_partial_eval           [ OK ]
    (5/7) Testing 3_quote                  [ OK ]
    (6/7) Testing 4_var_args               [ OK ]
    (7/7) Testing 5_var_args               [ OK ]
    --
    Passed  7
    Failed  0

### Mac OS X build

You should have `homebrew` installed.

    $ make macosx_deps
    $ make with_brew
