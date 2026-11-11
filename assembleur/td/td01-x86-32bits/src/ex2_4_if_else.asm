; TD1 2.4 Prg2 — if (var1 <= var2) var3 = 10; else { var3 = 6; var4 = 7; }

bits 32

section .data
  var1 dd 5
  var2 dd 8

section .bss
  var3 resd 1
  var4 resd 1

section .text
  global _start
_start:
  mov eax, [var1]
If_:
  cmp eax, [var2]
  ja Else_              ; si var1 > var2 -> else
  mov dword [var3], 10  ; then : var3 <- 10
  jmp Endif_
Else_:
  mov dword [var3], 6   ; else : var3 <- 6
  mov dword [var4], 7   ;        var4 <- 7
Endif_:
  mov eax, 1            ; sortie propre
  xor ebx, ebx
  int 0x80
