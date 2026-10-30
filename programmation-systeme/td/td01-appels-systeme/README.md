# TD1 — Appels système

| Fichier | Exercice |
|---|---|
| `src/ex01_hello.c` | 1 : afficher `hello` avec l'appel système `write`, invoqué directement par `syscall(SYS_write, ...)` |
| `src/ex02_03_erreur.c` | 2-3 : provoquer une erreur (`write` sur un descripteur invalide → `EBADF`) et l'afficher avec `perror` |
| `src/ex04_my_perror.c` | 4 : réécrire `perror` à partir de `errno` et `strerror` |

```sh
make
./bin/ex01_hello
./bin/ex02_03_erreur     # write: Bad file descriptor
./bin/ex04_my_perror
```
