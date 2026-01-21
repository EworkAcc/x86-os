extern interrupt_handler

%macro no_error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
  push dword 0
  push dword %1
  jmp common_interrupt_handler
%endmacro

%macro error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
  push dword %1
  jmp common_interrupt_handler
%endmacro

common_interrupt_handler:

  push eax
  push ebx
  push ecx
  push edx
  push ebp
  push esi
  push edi

  call interrupt_handler

  pop edi
  pop esi
  pop ebp
  pop edx
  pop ecx
  pop ebx
  pop eax

  add esp, 8

  iret

no_error_code_interrupt_handler 0
no_error_code_interrupt_handler 1
no_error_code_interrupt_handler 2
no_error_code_interrupt_handler 3
no_error_code_interrupt_handler 4
no_error_code_interrupt_handler 5
no_error_code_interrupt_handler 6
no_error_code_interrupt_handler 7

error_code_interrupt_handler 8
no_error_code_interrupt_handler 9

error_code_interrupt_handler 10
error_code_interrupt_handler 11
error_code_interrupt_handler 12
error_code_interrupt_handler 13
error_code_interrupt_handler 14

no_error_code_interrupt_handler 15

no_error_code_interrupt_handler 16
error_code_interrupt_handler 17
no_error_code_interrupt_handler 18
no_error_code_interrupt_handler 19
no_error_code_interrupt_handler 20
no_error_code_interrupt_handler 21

no_error_code_interrupt_handler 22
no_error_code_interrupt_handler 23
no_error_code_interrupt_handler 24
no_error_code_interrupt_handler 25
no_error_code_interrupt_handler 26
no_error_code_interrupt_handler 27

no_error_code_interrupt_handler 28
no_error_code_interrupt_handler 29
no_error_code_interrupt_handler 30

no_error_code_interrupt_handler 31
no_error_code_interrupt_handler 32

no_error_code_interrupt_handler 33
