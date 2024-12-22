#!/bin/bash

for test in $(ls *.s); do
    name="${test%.s}"
    ./dasm $test --lex > ./tests/$name.lex;
    ./dasm $test --elf > ./tests/$name.o;

    git --no-pager diff ./tests/$name.lex;
    git --no-pager diff ./tests/$name.o;
done
