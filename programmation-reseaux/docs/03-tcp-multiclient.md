# 3. Serveur TCP multi-client

Un serveur qui fait `accept` puis boucle sur `recv` ne sert qu'**un** client : `recv` bloque, les autres
attendent. Trois solutions.

| Méthode | Principe | Inconvénient |
|---|---|---|
| `fork()` par client | un processus par connexion | les processus ne partagent pas la mémoire : difficile de diffuser à tous |
| un thread par client | mémoire partagée | la liste des clients doit être protégée par un mutex |
| **multiplexage** (`select`, `poll`, `epoll`) | un seul processus attend que l'un des descripteurs soit prêt | il faut gérer l'état de chaque client à la main |

## `poll`

```c
struct pollfd fds[N];          // { fd, events = POLLIN, revents rempli par poll }
int n = poll(fds, nfds, -1);   // -1 : attente illimitée
```

Au réveil, on parcourt `fds` :

- socket d'écoute `POLLIN` → `accept`, ajouter le nouveau descripteur ;
- socket client `POLLIN` → `recv` ; `0` ou erreur → fermer et retirer du tableau ;
- `POLLHUP` / `POLLERR` → déconnexion.

`poll` n'a pas la limite de 1024 descripteurs de `select` (`FD_SETSIZE`) ; `epoll` (Linux) passe mieux à
l'échelle quand il y a des milliers de clients.

## Lire un gros fichier (TD3, ex. 2)

Le fichier arrive en plusieurs segments : boucler sur `recv` jusqu'à `0`, et écrire chaque morceau.
Pour un protocole à messages (chat), garder par client un **tampon** où s'accumulent les octets reçus,
et n'en extraire que des messages complets.

## Pièges

- Un client lent qui ne lit plus peut bloquer le serveur sur `send` : sockets non bloquantes
  (`O_NONBLOCK`) et file d'envoi par client.
- Retirer un client du tableau pendant qu'on le parcourt : décaler l'indice, ou marquer et fermer à la fin
  du tour.
- Tout ce que les clients envoient est non fiable : limiter les tailles, valider les commandes,
  neutraliser les caractères de contrôle avant de les renvoyer aux autres.
