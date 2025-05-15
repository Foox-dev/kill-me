; filepath: hello.asm
section .data
    msg db "Hello, World!", 10  ; 10 is newline
    msglen equ $ - msg

section .text
    global _start

_start:
    ; write syscall
    mov rax, 1          ; syscall number for write
    mov rdi, 1          ; file descriptor (stdout)
    mov rsi, msg        ; message to write
    mov rdx, msglen     ; message length
    syscall

    ; exit syscall
    mov rax, 60         ; syscall number for exit
    mov rdi, 0          ; exit code 0
    syscall