extern $printf:proc

.rdata
fmt db "hello assembler world, %d arguments  given", 0ah, 0
.code
main proc
    sub rsp, 40
    mov edx, ecx
    lea rcx, [rip + fmt]
    call $printf
    mov eax, 0
    add rsp, 40
    ret
main endp

.drectve SEGMENT
  db ' -defaultlib:crt64'
  db ' -defaultlib:kernel32'
end
