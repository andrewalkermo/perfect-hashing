#!bin/bash

gcc -o main src/main.c && ./main < tests/in.txt > tests/out_test.txt && diff -w tests/out_test.txt tests/out.txt && echo "OK" || exit 1
gcc -o main src/main.c && ./main < tests/in_full.txt > tests/out_test.txt && diff -w tests/out_test.txt tests/out_full.txt && echo "OK" || exit 1