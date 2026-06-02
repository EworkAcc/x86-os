bits 32
start:
  call load_base
load_base:
  pop esi
  mov edi, esi

  ; mkdir /tmp
  mov eax, 453
  lea ebx, [edi + dir_path - load_base]
  mov ecx, dir_path_len
  int 0x80

  ; create /tmp/a
  mov eax, 450
  lea ebx, [edi + file_a_path - load_base]
  mov ecx, file_a_path_len
  int 0x80

  ; open /tmp/a with O_RDWR|O_TRUNC
  mov eax, 5
  lea ebx, [edi + file_a_cstr - load_base]
  mov ecx, 0x202
  int 0x80
  mov ebp, eax

  ; write baseline
  mov eax, 4
  mov ebx, ebp
  lea ecx, [edi + msg_hello - load_base]
  mov edx, msg_hello_len
  int 0x80

  mov eax, 6
  mov ebx, ebp
  int 0x80

  ; chmod /tmp/a -> read only (0400)
  mov eax, 460
  lea ebx, [edi + file_a_path - load_base]
  mov ecx, file_a_path_len
  mov edx, 0x100
  int 0x80

  ; verify write-open fails
  mov eax, 5
  lea ebx, [edi + file_a_cstr - load_base]
  mov ecx, 0x1
  int 0x80
  cmp eax, -1
  jne open_write_unexpected

  mov eax, 4
  mov ebx, 1
  lea ecx, [edi + msg_write_blocked - load_base]
  mov edx, msg_write_blocked_len
  int 0x80
  jmp after_open_check

open_write_unexpected:
  mov eax, 4
  mov ebx, 1
  lea ecx, [edi + msg_write_unexpected - load_base]
  mov edx, msg_write_unexpected_len
  int 0x80

after_open_check:
  ; open read-only and read
  mov eax, 5
  lea ebx, [edi + file_a_cstr - load_base]
  mov ecx, 0
  int 0x80
  mov ebp, eax

  mov eax, 3
  mov ebx, ebp
  lea ecx, [edi + read_buffer - load_base]
  mov edx, read_buffer_len
  int 0x80
  mov esi, eax

  mov eax, 6
  mov ebx, ebp
  int 0x80

  ; print header and readback
  mov eax, 4
  mov ebx, 1
  lea ecx, [edi + msg_read_header - load_base]
  mov edx, msg_read_header_len
  int 0x80

  mov eax, 4
  mov ebx, 1
  lea ecx, [edi + read_buffer - load_base]
  mov edx, esi
  int 0x80

  ; chmod back writable so rename/truncate path remains testable
  mov eax, 460
  lea ebx, [edi + file_a_path - load_base]
  mov ecx, file_a_path_len
  mov edx, 0x180
  int 0x80

  ; rename /tmp/a -> /tmp/b
  mov eax, 458
  lea ebx, [edi + file_a_path - load_base]
  mov ecx, file_a_path_len
  lea edx, [edi + file_b_path - load_base]
  mov esi, file_b_path_len
  int 0x80

  ; truncate /tmp/b to 5 bytes
  mov eax, 459
  lea ebx, [edi + file_b_path - load_base]
  mov ecx, file_b_path_len
  mov edx, 5
  int 0x80

  ; read /tmp/b and print
  mov eax, 5
  lea ebx, [edi + file_b_cstr - load_base]
  mov ecx, 0
  int 0x80
  mov ebp, eax

  mov eax, 3
  mov ebx, ebp
  lea ecx, [edi + read_buffer - load_base]
  mov edx, read_buffer_len
  int 0x80
  mov esi, eax

  mov eax, 6
  mov ebx, ebp
  int 0x80

  mov eax, 4
  mov ebx, 1
  lea ecx, [edi + msg_trunc_header - load_base]
  mov edx, msg_trunc_header_len
  int 0x80

  mov eax, 4
  mov ebx, 1
  lea ecx, [edi + read_buffer - load_base]
  mov edx, esi
  int 0x80

  ; list /tmp
  mov eax, 454
  lea ebx, [edi + dir_path - load_base]
  mov ecx, dir_path_len
  lea edx, [edi + list_buffer - load_base]
  mov esi, list_buffer_len
  int 0x80
  mov esi, eax

  mov eax, 4
  mov ebx, 1
  lea ecx, [edi + msg_list_header - load_base]
  mov edx, msg_list_header_len
  int 0x80

  mov eax, 4
  mov ebx, 1
  lea ecx, [edi + list_buffer - load_base]
  mov edx, esi
  int 0x80

  ; unlink /tmp/b
  mov eax, 455
  lea ebx, [edi + file_b_path - load_base]
  mov ecx, file_b_path_len
  int 0x80

  mov eax, 4
  mov ebx, 1
  lea ecx, [edi + msg_done - load_base]
  mov edx, msg_done_len
  int 0x80

  mov eax, 1
  int 0x80

  jmp $

msg_write_blocked:
  db "write-open blocked after chmod", 10
msg_write_blocked_len equ $ - msg_write_blocked

msg_write_unexpected:
  db "BUG: write-open unexpectedly succeeded", 10
msg_write_unexpected_len equ $ - msg_write_unexpected

msg_read_header:
  db "read /tmp/a:", 10
msg_read_header_len equ $ - msg_read_header

msg_trunc_header:
  db "read /tmp/b after truncate(5):", 10
msg_trunc_header_len equ $ - msg_trunc_header

msg_list_header:
  db "list /tmp:", 10
msg_list_header_len equ $ - msg_list_header

msg_done:
  db "filesystem syscall test done", 10
msg_done_len equ $ - msg_done

dir_path:
  db "/tmp"
dir_path_len equ $ - dir_path

file_a_path:
  db "/tmp/a"
file_a_path_len equ $ - file_a_path

file_b_path:
  db "/tmp/b"
file_b_path_len equ $ - file_b_path

file_a_cstr:
  db "/tmp/a", 0

file_b_cstr:
  db "/tmp/b", 0

msg_hello:
  db "hello layered fs", 10
msg_hello_len equ $ - msg_hello

read_buffer:
  times 128 db 0
read_buffer_len equ $ - read_buffer

list_buffer:
  times 128 db 0
list_buffer_len equ $ - list_buffer
