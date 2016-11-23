#!/bin/bash

TESTS=$(ls *.in |while read i; do basename -s\.in $i; done)
TOTAL=$(ls *.in |wc -l)
STUTTER=$1
PASSED=0
FAILED=0
TEST=1

for t in $TESTS; do
    printf "($TEST/$TOTAL) Testing %-25s" $t
    tmp=$(mktemp)
    $STUTTER -f $t.in >$tmp
    out=$(diff -Zu $tmp $t.out)
    if [ "x$?" != "x0" ]; then
        echo "[ FAILED ]"
        echo -e "$out"
        let "FAILED++"
    else
        echo "[ OK ]"
        let "PASSED++"
    fi
    rm -f $tmp
    let "TEST++"
done

echo "--"
echo -e "Passed\t$PASSED"
echo -e "Failed\t$FAILED"

exit $FAILED
