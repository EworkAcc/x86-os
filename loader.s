global loader
  MAGIC_NUMBER equ 0x1BADB002
  FLAGS equ 0x0
  CHECKSUM equ -MAGIC_NUMBER

global outb

global inb

KERNEL_STACK_SIZE equ 4096

  section .bss
  align 4
  kernel_stack:
    resb KERNEL_STACK_SIZE

  section .text:
  align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

  outb:
    mov al, [esp + 8]
    mov dx, [esp + 4]
    out dx, al
    ret

  inb:
    mov dx, [esp + 4]
    in al, dx 
    ret

  loader:
    mov esp, kernel_stack + KERNEL_STACK_SIZE
    extern main

  call main

  .loop:
   jmp .loop
