; TD1 2.3 — traduction d'expressions arithmétiques (neg, imul, cdq, idiv)

bits 32

section .data
    Xval dd 10
    Yval dd 50
    Zval dd 20
    Rval dd 0

    var1 dd 6
    var2 dd 17
    var3 dd 5
    var4 dd 0

section .text
    global _start
    _start:
    ; ---- 1) Rval = -Xval + (Yval - Zval)
    mov eax, [Xval]
    neg eax                 ; eax = -Xval
    mov ebx, [Yval]
    sub ebx, [Zval]         ; ebx = Yval - Zval
    add eax, ebx
    mov [Rval], eax         ; attendu : -10 + 30 = 20

    ; ---- 2) var4 = (var1 + var2) * var3
    mov eax, [var1]
    add eax, [var2]         ; eax = 23
    imul eax, [var3]        ; eax = 115 (résultat sur 32 bits)
    mov [var4], eax

    ; ---- 3) var4 = (var1 * -5) / (var2 % var3)
    mov eax, [var2]
    cdq                     ; étend eax sur edx:eax (signe)
    idiv dword [var3]       ; eax = quotient, edx = reste (var2 % var3)
    mov ecx, edx            ; ecx = var2 % var3 = 2

    mov eax, [var1]
    imul eax, eax, -5       ; eax = var1 * -5 = -30
    cdq
    idiv ecx                ; eax = -30 / 2 = -15
    mov [var4], eax

    ; ---- sortie propre
    mov eax, 1
    xor ebx, ebx
    int 0x80
