#!/bin/bash

for test in $(ls ./tests/*.s); do
    name=$(basename "$test")
    name="${name%.s}"
    ./dasm $test --lex > ./tests/$name.lex
    ./dasm $test --elf ./tests/$name.o
done

git --no-pager diff ./tests/
