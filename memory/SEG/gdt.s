global segments_load_gdt
global segments_load_registers

segments_load_gdt:
  lgdt [esp + 4]
  ret

segments_load_registers:
  mov ax, 0x10
  mov ds, ax
  mov ss, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  jmp 0x08:flush_cs 

flush_cs:
  ret
