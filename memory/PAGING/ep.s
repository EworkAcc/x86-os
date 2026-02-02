global enable_paging

enable_paging:
 mov eax, [esp+4]
 mov cr3, eax
 
 mov eax, cr4
 or eax, 0x00000010
 mov cr4, eax

 mov eax, cr0
 or eax, 0x80000001
 mov cr0, eax 

 ret
