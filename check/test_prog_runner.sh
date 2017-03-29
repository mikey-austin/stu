VALGRIND="/usr/bin/valgrind"
if [ -x $VALGRIND ]; then
    $VALGRIND -q --trace-children=yes --track-origins=yes --leak-check=full --error-exitcode=1 --tool=memcheck $@
else
    $@
fi
