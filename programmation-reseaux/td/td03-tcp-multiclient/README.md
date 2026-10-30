# TD3 — Client / serveur TCP

| Fichier | Exercices | Rôle |
|---|---|---|
| `scripts/gen_test_file.sh` | 1 | Génère `test.txt` (100000 × `coucou` puis `end`) |
| `src/download_client.c` | 2 | Télécharge la totalité du fichier : boucle sur `recv` jusqu'à 0 |
| `src/broadcast_server.c` | 3 | Serveur port 8000 multi-client : chaque message est renvoyé à tous les autres clients |

```sh
make
./scripts/gen_test_file.sh
cat test.txt | nc -l 8000 -N &  ./bin/download_client
./bin/broadcast_server &  nc localhost 8000   # dans plusieurs terminaux
```

Multiplexage du serveur avec `poll()` (un seul processus, pas de threads) : c'est la base du serveur IRC du TP.
