# TP noté — IRC

Serveur de chat TCP multi-client (port 8000) et son client en C.

## Organisation

```
include/            en-têtes partagés
src/common/         code commun client + serveur
  net.c             stop(), création socket / bind / connect, envoi complet (send_all), lecture par ligne
  protocol.c        découpage d'une ligne en commande + arguments
src/echo/           partie 1 (ex. 1-16) : echo TCP repris du TD2
  tcpserver.c  tcpclient.c
src/server/         partie 2 + fonctionnalités
  main.c            boucle poll() : accept des nouveaux clients, lecture, déconnexions
  clients.c         table des clients connectés (fd, pseudo, salon), unicité des pseudos
  accounts.c        /register, /unregister, mots de passe, sauvegarde dans data/accounts.txt
  channels.c        /join, modérateur = créateur, /kick
  commands.c        dispatch : /nickname /exit /date /mp /alerte /send /red /blue /green ...
src/client/         partie 3
  main.c            arguments IP + port, demande du pseudo, poll() sur stdin + socket
  ntp.c             heure de réception via NTP (code du TD1)
  ui.c              horodatage, couleurs ANSI, alerte (\a + gras/rouge), écriture des fichiers reçus
tests/              scripts de test avec nc
rapport.md          réponses aux questions et choix de conception
```

| Exercice | Fichiers |
|---|---|
| 1-16 echo TCP | `src/echo/` |
| 17 serveur multi-client | `src/server/main.c`, `clients.c` |
| 18 client (IP/port, pseudo, heure NTP) | `src/client/` |
| 19 pseudos (`/nickname`, `/register`, `/unregister`) | `clients.c`, `accounts.c` |
| 20 `/exit`, `/date` | `commands.c` |
| 21 `/mp` | `commands.c` |
| 22 `/alerte` | `commands.c`, `src/client/ui.c` |
| 23 `/send` | `commands.c`, `src/client/ui.c` |
| 24 couleurs, salons, `/kick` | `commands.c`, `channels.c` |

## Protocole

Texte, une ligne terminée par `\n` par message. Une ligne qui commence par `/` est une commande, sinon c'est un message diffusé (préfixé par le pseudo par le serveur).

Pour `/send pseudo fichier`, le client envoie l'en-tête `/send <pseudo> <nom> <taille>\n` puis `<taille>` octets ; le serveur relaie l'en-tête et les octets au destinataire.

## Compilation et tests

```sh
make
./bin/irc_server 8000
./bin/irc_client 127.0.0.1 8000
make test
```
