# Programmation réseau

Programmation des sockets en C (3A STI, J. Briffaut) : UDP, TCP, serveur multi-client et TP noté IRC.

| Dossier | Contenu |
|---|---|
| `docs/` | Notes de cours (sockets, UDP/TCP, multiplexage) |
| `td/td01-client-ntp/` | Client NTP en UDP : `socket`, `gethostbyname`, `connect`, `read`/`write`, `ntohl` |
| `td/td02-udp-tcp/` | Ping-pong UDP (`bind`, `sendto`/`recvfrom`), echo TCP mono-client (`listen`, `accept`) |
| `td/td03-tcp-multiclient/` | Téléchargement d'un gros fichier (lecture en boucle) et serveur TCP multi-client (diffusion) |
| `tp/tp01-irc-note/` | TP noté : serveur et client de chat type IRC (pseudos, commandes, MP, fichiers, salons), [rapport](tp/tp01-irc-note/rapport.md) |

Les TD vont du plus simple au plus complet, et le TP réutilise leur code :

```
TD1 client NTP ──────────────────────────────┐
TD2 echo TCP ──► TP partie 1 (ex. 1-16)       ├──► TP client IRC (heure de réception via NTP)
TD3 serveur multi-client ──► TP partie 2 (ex. 17) ──► TP serveur IRC
```

## Avancement

| Travail | Statut |
|---|---|
| TD1 — client NTP (dont version durcie) | ✅ |
| TD2 — UDP ping-pong, TCP echo | ✅ |
| TD3 — téléchargement, serveur multi-client `poll` | ✅ |
| TP noté IRC — ex. 1 à 24, tests automatiques | ✅ |

## Compilation

```sh
make                              # compile tous les TD et le TP
make -C td/td02-udp-tcp
make -C tp/tp01-irc-note test     # tests du TP
make clean
```

Chaque TD produit un exécutable par fichier `src/*.c` dans son dossier `bin/`.
