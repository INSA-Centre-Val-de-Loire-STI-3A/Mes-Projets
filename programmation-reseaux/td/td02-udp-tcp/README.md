# TD2 — Client / serveur UDP et TCP

| Fichier | Exercices | Rôle |
|---|---|---|
| `src/udpserver.c` | 1-7 | Écoute 127.0.0.1:1234, affiche le message et l'émetteur, attend 1 s, répond `PONG` |
| `src/udpclient.c` | 8-13 | Envoie `PING` en boucle et affiche la réponse |
| `src/tcpserver.c` | 14-23 | Serveur echo mono-client (`listen(5)`, `accept`, 1000 `recv`/`send`) |
| `src/tcpclient.c` | 24-29 | Envoie `ECHO` 1000 fois et affiche la réponse |

```sh
make
./bin/udpserver &  ./bin/udpclient          # Ctrl+C pour arrêter
echo -n coucou | nc -4u -w0 localhost 1234

./bin/tcpserver &  ./bin/tcpclient          # 1000 « Recv : ECHO »
echo -n coucou | nc -4 -w0 localhost 1234
```

Vérification Wireshark (ex. 13 et 29) : interface `lo`, filtre `udp.port == 1234` ou `tcp.port == 1234`.
En TCP on voit la poignée de main (SYN, SYN-ACK, ACK) avant les données et la fermeture (FIN) à la fin ;
en UDP, seulement les datagrammes.

## Ex. 23 — que se passe-t-il avec `nc -w0` ?

`nc` envoie `coucou` puis ferme aussitôt la connexion. Le serveur affiche `coucou`, le renvoie,
puis le `recv` suivant **renvoie 0** : c'est la fin de flux, le client est parti. Si le serveur
ne teste pas ce cas, il boucle 1000 fois en affichant un message vide (le `0` n'est pas une
erreur, donc `stop` n'est jamais appelé), et ses `send` vers une connexion fermée provoquent
`EPIPE` / `SIGPIPE`, qui tue le processus.

Correction (dans `tcpserver.c`) :

```c
ssize_t n = recv(connfd, buf, BUFLEN, 0);
if (n == -1) stop("recv");
if (n == 0) { printf("Client déconnecté\n"); break; }   // fermer et attendre un autre client
```

et `send(..., MSG_NOSIGNAL)` pour recevoir l'erreur `EPIPE` au lieu du signal.

## Remarques

- `recvfrom` ne termine pas la chaîne : on écrit `buf[n] = '\0'` après avoir réservé un octet de plus.
- UDP n'a pas de connexion : le client vérifie que la réponse vient bien de l'adresse et du port du serveur.
- `SO_REUSEADDR` sur le serveur TCP permet de le relancer tout de suite (sinon `bind: Address already in use`
  pendant le `TIME_WAIT`).
- TCP est un **flux** : deux `send` peuvent arriver en un seul `recv`. Ici l'échange est alterné
  (le client attend la réponse avant de renvoyer), donc chaque `recv` contient un seul `ECHO`.
