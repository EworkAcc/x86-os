global enable_paging

enable_paging:
  ; load page directory
  mov eax, [esp+4]
  mov cr3, eax
  ; enable paging
  mov ebx, cr0
  or ebx, 0x80000000
  mov cr0, ebx
  ret
