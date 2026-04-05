bits 32
start:
  call load_base
load_base:
  pop esi

  mov eax, 1
  lea ebx, [esi + message - load_base]
  mov ecx, message_len
  int 0x80

  mov eax, 2
  int 0x80

  jmp $

message: 
  db "hello from user mode via syscall", 10
message_len equ $ - message
