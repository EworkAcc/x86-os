global enter_user_mode

enter_user_mode:
  cli

  mov eax, [esp + 4]

  mov dx, 0x23
  mov ds, dx
  mov es, dx
  mov fs, dx
  mov gs, dx

  push 0x23
  push 0x800000
  pushf
  push 0x1b
  push eax

  iret
