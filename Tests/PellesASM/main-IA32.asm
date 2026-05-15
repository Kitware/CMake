extern $printf:proc

.rdata
fmt db "hello assembler world, %d arguments  given", 0ah, 0
.code
main proc
    push ebp
    mov ebp, esp
    push dword ptr [ebp + 8]
    lea eax, [fmt]
    push eax
    call $printf
    add esp, 8
    mov eax, 0
    mov esp, ebp
    pop ebp
    ret
main endp

.drectve SEGMENT
  db ' -defaultlib:crt'
  db ' -defaultlib:kernel32'
end
