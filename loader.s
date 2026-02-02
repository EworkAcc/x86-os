global kernel_stack
global loader
  MAGIC_NUMBER equ 0x1badb002
  ALIGN_MODULES equ 0x00000001
  CHECKSUM equ -(MAGIC_NUMBER + ALIGN_MODULES)
  FLAGS equ 0x0

KERNEL_STACK_SIZE equ 4096

  section .bss
  align 4
  kernel_stack:
    resb KERNEL_STACK_SIZE

  section .text
  align 4
    dd MAGIC_NUMBER
    dd ALIGN_MODULES
    dd CHECKSUM
    dd FLAGS

  loader:
    mov esp, kernel_stack + KERNEL_STACK_SIZE
    
    push ebx

    extern kmain

    call kmain

  .loop:
    jmp .loop
