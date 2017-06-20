VALGRIND=/usr/bin/valgrind
echo "=== Executing tests under Valgrind ==="


$VALGRIND --tool=memcheck --leak-check=full ./picco -S $1
#          --suppressions=tests.supp $1
