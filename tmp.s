.intel_syntax noprefix
.global _main
_main:
    push 2
    push 3
    push 4
    pop rdi
    pop rax
    mul rdi
    push rax
    push 1
    pop rdi
    pop rax
    sub rax, rdi
    push rax
    pop rdi
    pop rax
    add rax, rdi
    push rax
    push 2
    push 4
    push 1
    pop rdi
    pop rax
    sub rax, rdi
    push rax
    pop rdi
    pop rax
    add rax, rdi
    push rax
    pop rdi
    pop rax
    mul rdi
    push rax
    pop rax
    ret
