# TD1 — Assembleur x86 : 8086 et mode protégé 32 bits

| Fichier | Exercice | Notions |
|---|---|---|
| `src/hello32.asm` | Exemple du cours | Appels système `write` et `exit` par `int 0x80` |
| — | 1.1 Adressage logique → physique | Mode réel : `physique = segment × 16 + offset` |
| `src/ex1_2_modes_adressage.asm` | 1.2 Modes d'adressage | Immédiat, direct, registre, indirect, basé-indexé ; code 16 bits (`bits 16`) |
| `src/ex2_1_drapeaux.asm` | 2.1 UAL et drapeaux | Débordement de `add` / `sub` sur 32 bits : CF, ZF, SF, OF |
| `src/ex2_2_registres_memoire.asm` | 2.2 Programme assembleur | `mov`, `movzx`, adressage `[esi+ebp+10h]` |
| `src/ex2_3_operations.asm` | 2.3 Opérations | `neg`, `imul`, `cdq` + `idiv` (quotient dans EAX, reste dans EDX) |
| `src/ex2_4_if_else.asm` | 2.4 Prg2 | `if` / `else` : `cmp` + saut conditionnel non signé (`ja`) |

À faire : 2.4 Prg1, Prg3, Prg4 (`while`) · 2.5 comparaison de chaînes · 2.6 recherche de caractère ·
2.7 nombres premiers et `movsd` · 2.8 structure de données · 2.9 inversion des bits · 3 tri à bulle.

```sh
make
objdump -m i8086 -D bin/ex1_2_modes_adressage   # code machine 16 bits
ddd bin/ex2_1_drapeaux                          # pas à pas, menu Status > Registers
./bin/ex2_3_operations; echo $?                 # sortie propre : 0
```

`ex1_2_modes_adressage` (instructions 16 bits) et `ex2_2_registres_memoire` (pointeurs non initialisés)
servent à étudier le code machine : les exécuter provoque une erreur de segmentation.
