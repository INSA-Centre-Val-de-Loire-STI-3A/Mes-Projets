# TD3 — Client / serveur TCP

| Fichier | Exercices | Rôle |
|---|---|---|
| `scripts/gen_test_file.sh` | 1 | Génère `test.txt` (100000 × `coucou` puis `end`, 700 004 octets) |
| `src/download_client.c` | 2 | Télécharge la totalité du fichier : boucle sur `recv` jusqu'à 0 |
| `src/broadcast_server.c` | 3 | Serveur port 8000 multi-client : chaque message est renvoyé à tous les autres clients |

```sh
make
./scripts/gen_test_file.sh
cat test.txt | nc -l 8000 -N &  ./bin/download_client      # → download.txt
cmp test.txt download.txt && echo identiques

./bin/broadcast_server &  nc localhost 8000               # dans plusieurs terminaux
```

## Ex. 2 — pourquoi une boucle ?

TCP transporte un **flux d'octets**, pas des messages : le fichier arrive découpé selon la taille
des segments et des tampons (souvent quelques dizaines de `recv` pour 700 Ko). Un seul `recv`
ne lit que le premier morceau. Le client boucle donc jusqu'à ce que `recv` renvoie **0** : `nc -N`
ferme la connexion quand il a tout envoyé, c'est la fin du fichier. De même, `write` peut écrire
moins que demandé : `write_all` boucle aussi.

## Ex. 3 — plusieurs clients sans thread

Le serveur utilise `poll()` : un seul processus surveille la socket d'écoute et toutes les sockets
clientes, et se réveille dès que l'une d'elles est prête.

- socket d'écoute prête → `accept` d'un nouveau client, ajouté au tableau ;
- socket cliente prête → `recv` ; `0` ou erreur = déconnexion (la case est remplacée par la dernière),
  sinon le message est envoyé à tous les **autres** clients.

Alternatives : un processus par client (`fork`, mais les processus ne partagent pas la liste des
clients) ou un thread par client (liste partagée à protéger par un mutex). `poll` évite les deux
problèmes, et c'est la base du serveur IRC du TP.
