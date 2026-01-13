global enter_user_mode

enter_user_mode:
  cli

  mov eax, [esp + 4]

  mov ax, 0x23
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  push 0x23
  push 0x800000
  pushf
  push 0x1b
  push eax

  iret
