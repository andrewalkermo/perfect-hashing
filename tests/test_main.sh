#!bin/bash
result=0
for input in tests/*.in; do
  output=${input%.in}.out
  rm -f registros
  gcc -o main src/main.c
  ./main < $input > test.out
  diff -w -u1 --color=auto --palette='ad=1;3;38;5;154:de=1;3;38;5;9' test.out $output && echo "OK" || result=1
done
rm -f registros
rm -f test.out
rm -f main
return $result