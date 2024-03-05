global _start

extern LibNasm1Func

section .text
_start:
  xor rax, rax
  call LibNasm1Func
  cmp rax, 1
  jne err

  mov rax, 60
  xor rdi, rdi
  syscall

err:
  mov rax, 60
  mov rdi, 1
  syscall
