#!/bin/bash

TESTS=$(ls *.in |while read i; do basename -s\.in $i; done)
TOTAL=$(ls *.in |wc -l)
PASSED=0
FAILED=0
TEST=1
VALGRIND=

while getopts "mf:" opt; do
    case $opt in
        f)
            STUTTER=$OPTARG
            ;;
        m)
            VALGRIND="/usr/bin/valgrind -q --error-exitcode=1 --leak-check=full --errors-for-leak-kinds=all --show-leak-kinds=all --tool=memcheck"
            ;;
        \?)
            echo k"Invalid option: -$OPTARG" >&2
            ;;
  esac
done

for t in $TESTS; do
    printf "(%2s/$TOTAL) Testing %-25s" $TEST $t
    tmp=$(mktemp)
    errtmp=$(mktemp)
    $VALGRIND $STUTTER -f $t.in >$tmp 2>$errtmp
    if [ "x$?" != "x0" ]; then
        echo "[ FAILED ]"
        echo "--"
        echo "Valgrind errors:"
        cat $errtmp
        echo "--"
        let "FAILED++"
    else
        out=$(diff -Zu $tmp $t.out)
        if [ "x$?" != "x0" ]; then
            echo "[ FAILED ]"
            echo -e "Command: $VALGRIND $STUTTER -f $t.in"
            echo -e "$out"
            let "FAILED++"
        else
            echo "[ OK ]"
            let "PASSED++"
        fi
    fi
    rm -f $tmp $errtmp
    let "TEST++"
done

echo "--"
echo -e "Passed\t$PASSED"
echo -e "Failed\t$FAILED"

exit $FAILED
