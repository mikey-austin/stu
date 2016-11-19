### Stutter; another lisp implementation

Stutter is a simple & minimal lisp implementation in C.

    $ make >/dev/null
    $ cat <<END |./stutter -f-
    (def 'a 100)
    (def 'b 200)
    (def 'c (+ a (* 2 b)))
    (eval (list 'quote a b c))
    END
    (100 200 500)

Or you can use the repl by specifying no arguments:

    $ ./stutter 
    stutter> (car (reverse '(1 2 3 4 5 6)))
    6
    stutter> (cdr (reverse '(1 2 3 4 5 6)))
    (5 4 3 2 1)
    stutter> ^D
    Bye!

### Mac OS X build

You should have `homebrew` installed.

    make macosx_deps
    make with_brew
