global loader
  MAGIC_NUMBER equ 0x1badb002
  ALIGN_MODULES equ 0x00000001
  FLAGS equ 0x0
  CHECKSUM equ -(MAGIC_NUMBER + ALIGN_MODULES)

KERNEL_STACK_SIZE equ 4096

  section .bss
  align 4
  kernel_stack:
    resb KERNEL_STACK_SIZE

  section .text
  align 4
    dd MAGIC_NUMBER
    dd ALIGN_MODULES
    dd FLAGS
    dd CHECKSUM

  loader:
    mov esp, kernel_stack + KERNEL_STACK_SIZE
    
    push ebx

    extern main

    call main

  .loop:
    jmp .loop
