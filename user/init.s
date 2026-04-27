bits 32
start:
  call load_base
load_base:
  pop esi
  mov edi, esi

  mov eax, 453
  lea ebx, [edi + dir_path - load_base]
  mov ecx, dir_path_len
  int 0x80

  mov eax, 4
  lea ebx, [edi + header - load_base]
  mov ecx, header_len
  int 0x80

  mov eax, 454
  lea ebx, [edi + root_path - load_base]
  mov ecx, root_path_len
  lea edx, [edi + list_buffer - load_base]
  mov esi, list_buffer_len
  int 0x80

  mov eax, 4
  lea ebx, [edi + list_buffer - load_base]
  mov ecx, eax
  int 0x80

  mov eax, 1
  int 0x80

  jmp $

header:
  db "root directory entries:", 10
header_len equ $ - header

dir_path:
  db "/tmp"
dir_path_len equ $ - dir_path

root_path:
  db "/"
root_path_len equ $ - root_path

list_buffer:
  times 128 db 0
list_buffer_len equ $ - list_buffer
