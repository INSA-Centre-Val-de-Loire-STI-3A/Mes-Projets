; TD1 2.2 — transferts registres / mémoire (analyse du code machine ;
; plante à l'exécution : ebx et esi ne pointent sur rien)

bits 32

section .text
  global _start
  _start:
    mov ax, 78

    mov ax, [ebx]

    movzx edx, ax
    add   edx, 0x7659

    mov cx, [esi+0x20]

    mov eax, ebx
    add eax, [esi]
    mov [esi+ebp], eax

    mov dl, al
    sub dl, 1
    mov [ebp+esi+0x10], dl
