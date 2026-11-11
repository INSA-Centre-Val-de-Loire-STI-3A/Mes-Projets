; TD2 2 — Aff_Bin() : afficher le contenu de EAX en binaire sur 32 caractères
; ex. 2026 -> 00000000000000000000011111101010

bits 32

section .data
    message     db 'Valeur en binaire : '
    len_message equ $ - message
    valeur      dd 2026

section .bss
    tampon      resb 33             ; 32 chiffres + saut de ligne

section .text
    global _start

; ---- programme principal
_start:
    mov eax, 4                      ; write(stdout, message, len_message)
    mov ebx, 1
    mov ecx, message
    mov edx, len_message
    int 0x80

    mov eax, [valeur]               ; paramètre de Aff_Bin : la variable dans EAX
    call Aff_Bin

    mov eax, 1                      ; exit(0)
    xor ebx, ebx
    int 0x80

; ---- Aff_Bin : entrée EAX = valeur à afficher ; aucun registre modifié
Aff_Bin:
    pushad                          ; sauvegarde des registres généraux

    mov edi, tampon
    mov ecx, 32                     ; 32 bits à traiter, du poids fort au poids faible
.bit_suivant:
    mov byte [edi], '0'
    shl eax, 1                      ; bit de poids fort -> CF
    adc byte [edi], 0               ; '0' + CF : '0' ou '1'
    inc edi
    loop .bit_suivant               ; ecx-- ; recommence tant que ecx != 0

    mov byte [edi], 10              ; saut de ligne

    mov eax, 4                      ; write(stdout, tampon, 33)
    mov ebx, 1
    mov ecx, tampon
    mov edx, 33
    int 0x80

    popad                           ; restauration des registres
    ret
