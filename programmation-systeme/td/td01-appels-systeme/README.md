# TD1 — Appels système

Appeler le noyau directement avec `syscall`, provoquer une erreur et la traiter avec `errno`.

| Fichier | Exercices |
|---|---|
| `src/ex01_hello.c` | 1 : afficher `hello` avec `syscall(SYS_write, ...)`, sans passer par la libc (`printf`) |
| `src/ex02_03_erreur.c` | 2-3 : écrire sur un descripteur invalide (fd 9) pour obtenir `EBADF`, puis l'afficher avec `perror` |
| `src/ex04_my_perror.c` | 4 : réécrire `perror` avec `errno` et `strerror` (`my_perror`) |

```sh
make
./bin/ex01_hello          # hello
./bin/ex02_03_erreur      # write: Bad file descriptor
./bin/ex04_my_perror      # write : Bad file descriptor
```

`strace ./bin/ex01_hello` montre l'appel `write(1, "hello\n", 6)` émis tel quel.

## À retenir

- `syscall` renvoie `-1` en cas d'échec et place le code d'erreur dans `errno`.
- `errno` doit être lu (ou copié) juste après l'appel qui a échoué : tout appel suivant,
  même `fprintf`, peut l'écraser. C'est pour cela que `my_perror` le sauvegarde d'abord.
