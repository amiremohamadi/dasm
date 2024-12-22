#!/bin/bash

for test in $(ls ./tests/*.s); do
    name=$(basename "$test")
    name="${name%.s}"

    echo -e "\033[32mRunning --lex for $test\033[0m"
    ./dasm $test --lex > ./tests/$name.lex

    echo -e "\033[32mRunning --elf for $test\033[0m"
    ./dasm $test --elf ./tests/$name.o
done

git --no-pager diff --exit-code ./tests/ || exit 1
git --no-pager diff --cached --exit-code ./tests/ || exit 1
