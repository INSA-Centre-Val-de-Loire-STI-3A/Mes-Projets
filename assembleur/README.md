# Assembleur x86

Architecture des ordinateurs et programmation en assembleur (3A STI, E. Ferrère) : 8086 en mode réel,
IA-32 en mode protégé, appels système Linux, passage de paramètres par la pile, x86_64.

| Dossier | Contenu |
|---|---|
| `td/td01-x86-32bits/` | Adressage 8086, drapeaux de l'UAL, transferts registres / mémoire, arithmétique, structures de contrôle |
| `td/td02-fonctions-pile/` | Appels système (`int 0x80`, `syscall`), fonctions avec paramètres sur la pile, récursivité, chaînes en x86_64 |

## Outils

```sh
sudo apt install nasm gcc-multilib gdb ddd
```

## Compilation

```sh
make                          # assemble tous les TD
make -C td/td01-x86-32bits
make clean
```

Chaque TD produit un exécutable par fichier `src/*.asm` non vide dans son dossier `bin/` :

| Source | Assemblage | Édition de liens |
|---|---|---|
| `*.asm` (32 bits) | `nasm -f elf32 -g -F dwarf` | `ld -m elf_i386` |
| `*_x64.asm` (64 bits) | `nasm -f elf64 -g -F dwarf` | `ld` |

## Déboguer

```sh
ddd bin/ex2_1_drapeaux                      # interface graphique du TD
gdb -q bin/ex2_1_drapeaux
(gdb) layout asm                            # code désassemblé
(gdb) layout regs                           # registres
(gdb) break _start
(gdb) run
(gdb) stepi                                 # une instruction
(gdb) info registers eflags                 # drapeaux CF, ZF, SF, OF…
(gdb) x/4xw &var1                           # 4 mots de 32 bits en hexa à l'adresse de var1
objdump -M intel -d bin/ex2_3_operations    # code machine en syntaxe Intel
```
