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

  mov eax, 450
  lea ebx, [edi + file_path - load_base]
  mov ecx, file_path_len
  int 0x80

  mov eax, 5
  lea ebx, [edi + file_cstr - load_base]
  mov ecx, 512
  int 0x80
  mov ebp, eax

  mov eax, 4
  mov ebx, ebp
  lea ecx, [edi + file_data - load_base]
  mov edx, file_data_len
  int 0x80


  mov eax, 6
  mov ebx, ebp
  int 0x80

  mov eax, 5
  lea ebx, [edi + file_cstr - load_base]
  mov ecx, 1024
  int 0x80
  mov ebp, eax

  mov eax, 4
  mov ebx, ebp
  lea ecx, [edi + file_data2 - load_base]
  mov edx, file_data2_len
  int 0x80

  mov eax, 5
  lea ebx, [edi + file_cstr - load_base]
  mov ecx, 0
  int 0x80
  mov ebp, eax


  mov eax, 19
  mov ebx, ebp
  mov ecx, 0
  mov edx, 0
  int 0x80

  mov eax, 3
  mov ebx, ebp
  lea ecx, [edi + file_buffer - load_base]
  mov edx, file_buffer_len
  int 0x80
  mov esi, eax

  mov eax, 4
  mov ebx, 1
  lea ecx, [edi + header - load_base]
  mov edx, header_len
  int 0x80

  mov eax, 4
  mov ebx, 1
  lea ecx, [edi + file_buffer - load_base]
  mov edx, esi
  int 0x80

  mov eax, 6
  mov ebx, ebp
  int 0x80

  mov eax, 1
  int 0x80

  jmp $

header:
  db "fd readback:", 10
header_len equ $ - header

dir_path:
  db "/tmp"
dir_path_len equ $ - dir_path

file_path:
  db "/tmp/a"
file_path_len equ $ - file_path

file_cstr:
  db "/tmp/a", 0

file_data:
  db "hello layered fs", 10
file_data_len equ $ - file_data

file_data2:
  db "+append", 10
file_data2_len equ $ - file_data2

file_buffer:
  times 128 db 0
file_buffer_len equ $ - file_buffer
