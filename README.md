### Stutter; another lisp implementation

Stutter is a simple & minimal lisp implementation in C.

    $ make >/dev/null
    $ cat <<END |./stutter -f-
    (def a 100)
    (def b 200)
    (def c (+ a (* 2 b)))
    (eval (list 'quote a b c))
    END
    (100 200 500)

Or you can use the repl by specifying no arguments:

    $ ./stutter
    stutter> (def make-adder (lambda (a) (lambda (b) (+ a b))))
    nil
    stutter> (def add-two (make-adder 2))
    nil
    stutter> (add-two 2)
    4
    stutter> ^D
    Bye!

### Mac OS X build

You should have `homebrew` installed.

    make macosx_deps
    make with_brew
