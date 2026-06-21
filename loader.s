global kernel_stack
global loader
  MAGIC_NUMBER equ 0x1badb002
  ALIGN_MODULES equ 0x00000001
  MEMORY_INFO equ 0x00000002
  VIDEO_MODE equ 0x00000004
  FLAGS equ ALIGN_MODULES | MEMORY_INFO | VIDEO_MODE
  CHECKSUM equ -(MAGIC_NUMBER + FLAGS)

  VIDEO_MODE_TYPE_GRAPHICS equ 0
  VIDEO_WIDTH equ 1024
  VIDEO_HEIGHT equ 768
  VIDEO_DEPTH equ 32

KERNEL_STACK_SIZE equ 4096

  section .bss
  align 4
  kernel_stack:
    resb KERNEL_STACK_SIZE

  section .text
  align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM
    dd 0
    dd 0
    dd 0
    dd 0
    dd 0
    dd VIDEO_MODE_TYPE_GRAPHICS
    dd VIDEO_WIDTH
    dd VIDEO_HEIGHT
    dd VIDEO_DEPTH

  loader:
    mov esp, kernel_stack + KERNEL_STACK_SIZE
    
    push ebx

    extern kmain

    call kmain

  .loop:
    jmp .loop
