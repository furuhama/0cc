.intel_syntax noprefix
.global plus, _main

plus:
        add rsi, rdi
        mov rax, rsi
        ret

_main:
        mov rdi, 20
        mov rsi, 22
        call plus
        ret
