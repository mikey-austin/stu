### Stu; another lisp implementation [![Build Status](https://travis-ci.org/mikey-austin/stu.svg?branch=master)](https://travis-ci.org/mikey-austin/stu)

Stu is a simple & minimal lisp implementation in C. To build, you need the following installed:
- autoconf, automake & libtool
- valgrind (optional but highly recommended)
- libedit-devel

The following commands can be performed to develop stu:

    $ autoreconf -i && ./configure ALLOCATOR=system CFLAGS="-O0 -g" --disable-shared && make
    ...
    $ cat <<END |./src/repl/stu -f-
    > (def a 100)
    > (def b 200)
    > (def c (+ a (* 2 b)))
    > (eval (list 'quote a b c))
    > END
    (100 200 500)

Or you can use the repl by specifying no arguments:

    $ ./src/repl/stu
    stu> (def make-adder (λ (a) (λ (b) (+ a b))))
    nil
    stu> (def add-two (make-adder 2))
    nil
    stu> (add-two 2)
    4
    stu> ^D
    Bye!

### Documentation

Online manual pages are generated from both _docs/*.pod_ files and various headers in the source. The HTML docs are automatically made available at the [stu github page](https://mikey-austin.github.io/stu/stu.html). The equivalent roff manual pages are installed during a _sudo make install_.

### Tests

The test suite uses valgrind if installed (which you should).

    $ make check
