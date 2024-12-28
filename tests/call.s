global _start

main:
syscall

_start:
mov rax, 60
mov rdi, 69
call main

