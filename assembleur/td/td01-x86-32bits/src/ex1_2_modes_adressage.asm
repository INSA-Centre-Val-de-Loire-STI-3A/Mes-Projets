; TD1 1.2 — modes d'adressage du 8086 (code 16 bits, à désassembler, pas à exécuter)
; objdump -m i8086 -D bin/ex1_2_modes_adressage

bits 16

section .bss
  val: resw 1
  chr: resb 1

section .text
  global _start
  _start:
    mov ax, 100
    add ax, word [val]
    mov ax, bx
    mov byte [chr], 'H'
    sub ax, [si]
    sub ax, [si+bp]

