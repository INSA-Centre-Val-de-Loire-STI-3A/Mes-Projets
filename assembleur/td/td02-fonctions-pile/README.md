# TD2 — Appels système, fonctions et pile

| Fichier | Exercice | Notions |
|---|---|---|
| `src/ex2_aff_bin.asm` | 2 Affichage binaire ✅ | `Aff_Bin()` : afficher EAX en binaire sur 32 caractères (`shl` + `adc`, `loop`, `pushad` / `popad`, `write`) |
| `src/ex3_palindrome.asm` | 3 Palindrome | `int Palin(char *)` : lecture au clavier (`read`, ≤ 20 caractères), paramètre et retour par la pile |
| `src/ex4_crc16.asm` | 4 CRC16 Modbus | `short CRC16(char *trame, int ltrame)` : XOR, décalages, polynôme `0xA001` ; trame `01 03 04 01 17 00 83` → CRC `0x6A0A` |
| `src/ex5_factorielle.asm` | 5 Factorielle récursive | Tableau `n, n(n-1), …, n!` ; paramètres par registres, observation de la pile (`call` / `ret`) |
| `src/ex6_chaines_x64.asm` | 6 Chaînes en x86_64 | `asm_copy`, `asm_find`, `asm_strlen`, `asm_strcpy`, `asm_strcat` ; appels système par `syscall` |

```sh
make            # n'assemble que les sources non vides
./bin/ex2_aff_bin
```

## Rappels

| | 32 bits (`int 0x80`) | 64 bits (`syscall`) |
|---|---|---|
| Numéro de l'appel | `eax` : `read` 3, `write` 4, `exit` 1 | `rax` : `read` 0, `write` 1, `exit` 60 |
| Arguments | `ebx`, `ecx`, `edx` | `rdi`, `rsi`, `rdx` |
| Valeur de retour | `eax` | `rax` |

Passage par la pile en 32 bits : l'appelant empile les paramètres (`push`) puis fait `call` ;
la fonction installe son cadre (`push ebp` / `mov ebp, esp`) et lit le premier paramètre en `[ebp+8]`,
le second en `[ebp+12]`. Au retour, l'appelant dépile les paramètres (`add esp, 4 × nb`).

Pour appeler les fonctions de l'exercice 6 depuis du C (convention System V : arguments dans
`rdi`, `rsi`, `rdx`, retour dans `rax`) : les déclarer `global` et les lier avec `gcc main.c ex6.o`.
