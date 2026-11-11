; Exemple du cours : afficher un message avec l'appel système write (int 0x80, 32 bits)

bits 32

section .data
    message db 'Hello insa-cvl!', 10
    len     equ $ - message     ; taille calculée par nasm (16 octets)

section .text
    global _start
    _start:
        mov eax, 4          ; numéro de write
        mov ebx, 1          ; descripteur stdout
        mov ecx, message
        mov edx, len        ; taille du message
        int 0x80            ; appel noyau

        mov eax, 1          ; numéro de exit
        mov ebx, 0          ; code retour
        int 0x80            ; appel noyau
