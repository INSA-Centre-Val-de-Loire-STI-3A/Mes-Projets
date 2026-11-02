# Programmation système

Programmation système en C sous Linux (3A STI) : appels système, fichiers, processus et redirections.

| Dossier | Contenu |
|---|---|
| `docs/` | Notes de cours |
| `td/td01-appels-systeme/` | `syscall`, `errno`, `perror` / `strerror` |
| `td/td01bis-cp-benchmark/` | Copie de fichier et influence de la taille du tampon |
| `td/td02-mini-ls/` | Parcours d'un répertoire, `stat` |
| `td/td03-fs-inodes/` | Système de fichiers et inodes |
| `td/td04-fichiers/` | Entrées-sorties sur fichiers |
| `td/td05-verrous/` | Verrous sur fichiers |
| `td/td06-processus/` | Création de processus (`fork`, `wait`) |
| `td/td07-zombies-exec/` | Processus zombies, recouvrement avec `exec` |
| `td/td08-redirections/` | Redirections et tubes (`dup2`, `pipe`) |
| `tp/tp01-mini-glibc/` | TP : réimplémentation d'une partie de la libc, `mini_cat`, mini-shell |

## Compilation

```sh
make -C td/td01-appels-systeme
make -C td/td01-appels-systeme clean
```

Chaque TD produit un exécutable par fichier `src/*.c` dans son dossier `bin/`.
