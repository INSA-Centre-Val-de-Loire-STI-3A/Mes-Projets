; TD1 2.1 B — évolution de EAX et des drapeaux CF / ZF / SF (pas à pas dans ddd ou gdb)

bits 32

section .data
    var1 dd 0x10000000
    var2 dd 0x20000000

section .text
    global _start
    _start:
   mov eax, [var1]
   add eax, [var2]
   mov eax, 0xFFFFFFFF
   add eax, 1
   sub eax, 1

   mov eax, 1          ; sortie propre (exit)
   xor ebx, ebx
   int 0x80
