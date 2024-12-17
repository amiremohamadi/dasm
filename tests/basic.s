mov rax, 60 ; sys_exit
mov rdi, 69

jmp end

here:
mov rdi, 85
syscall

end:
jmp here
