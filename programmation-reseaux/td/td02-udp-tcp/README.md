# TD2 — Client / serveur UDP et TCP

| Fichier | Exercices | Rôle |
|---|---|---|
| `src/udpserver.c` | 1-7 | Écoute 127.0.0.1:1234, affiche le message et l'émetteur, répond `PONG` |
| `src/udpclient.c` | 8-13 | Envoie `PING` en boucle et affiche la réponse |
| `src/tcpserver.c` | 14-23 | Serveur echo mono-client (`listen(5)`, `accept`, 1000 `recv`/`send`) |
| `src/tcpclient.c` | 24-29 | Envoie `ECHO` 1000 fois et affiche la réponse |

```sh
make
./bin/udpserver &  ./bin/udpclient
echo -n coucou | nc -4u -w0 localhost 1234

./bin/tcpserver &  ./bin/tcpclient
echo -n coucou | nc -4 -w0 localhost 1234
```

Ex. 23 : la réponse est à noter ici (que se passe-t-il quand le client `nc` se déconnecte ? → `recv` renvoie 0).
