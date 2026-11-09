# 2. Client / serveur UDP et TCP

## Serveur UDP

```
socket(AF_INET, SOCK_DGRAM) → bind(adresse:port) → boucle { recvfrom → traitement → sendto(émetteur) }
```

`bind` associe la socket à une adresse locale. Le client n'a pas besoin de `bind` : le noyau lui
choisit un port éphémère au premier `sendto`.

## Serveur TCP

```
socket(SOCK_STREAM) → bind → listen(n) → accept → recv / send … → close
```

- `listen(fd, n)` passe la socket en écoute ; `n` = taille de la file des connexions en attente.
- `accept` renvoie une **nouvelle** socket, dédiée à ce client ; la socket d'écoute continue d'accepter.
- Le client fait `socket` puis `connect` : poignée de main SYN, SYN-ACK, ACK.

## TCP est un flux

TCP garantit l'ordre et l'arrivée, mais **ne conserve pas les frontières des messages** : deux `send`
peuvent arriver en un `recv`, un `send` peut arriver en plusieurs. Il faut donc un protocole qui délimite
les messages (fin de ligne `\n`, ou taille annoncée) et une boucle de lecture.

## Valeurs de retour

| Retour de `recv` | Sens |
|---|---|
| `> 0` | nombre d'octets reçus (la chaîne n'est **pas** terminée par `\0`) |
| `0` | le pair a fermé la connexion (fin de flux) |
| `-1` | erreur (`errno`) ; `EINTR` = interrompu par un signal, réessayer |

`send` peut envoyer moins que demandé : boucler (`send_all`). Écrire dans une connexion fermée par le
pair lève `SIGPIPE`, qui tue le processus : `send(..., MSG_NOSIGNAL)` ou `signal(SIGPIPE, SIG_IGN)`.

## Erreurs fréquentes

- `bind: Address already in use` en relançant un serveur : la connexion précédente est en `TIME_WAIT`.
  `setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))` avant `bind`.
- Ne pas tester `recv() == 0` : boucle infinie sur un client parti (TD2, ex. 23).
- Afficher le tampon reçu avec `%s` sans ajouter `\0`.

## Outils

- `nc -4u localhost 1234` (UDP), `nc localhost 1234` (TCP), `nc -l 8000` (serveur).
- `ss -tlnp` / `ss -ulnp` : sockets en écoute.
- Wireshark sur `lo`, filtre `tcp.port == 1234`.
