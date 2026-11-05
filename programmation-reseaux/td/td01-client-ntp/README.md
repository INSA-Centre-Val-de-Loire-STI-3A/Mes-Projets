# TD1 — Socket et client NTP

Interroger un serveur NTP (`fr.pool.ntp.org`, UDP port 123) et afficher l'heure reçue.

| Fichier | Exercices |
|---|---|
| `src/ntpclient-udp.c` | 1-24 : `error()`, struct `ntp_packet` (48 octets), macros `SET_LI/VN/MODE`, `socket`, `gethostbyname`, `connect`, `write`/`read`, `ntohl`, `NTP_TIMESTAMP_DELTA`, `ctime` |
| `src/ntpclient-udp-sendto.c` | 25-26 : serveur passé en argument, `sendto`/`recvfrom` à la place de `connect` + `read`/`write` |
| `src/ntpclient-udp-secure.c` | version durcie : vérification de la réponse (voir plus bas) |

```sh
make
./bin/ntpclient-udp                         # interroge fr.pool.ntp.org
./bin/ntpclient-udp-sendto fr.pool.ntp.org  # serveur au choix
```

Vérification Wireshark (ex. 18) : filtre `udp.port == 123`.

## Points de sécurité

- `gethostbyname` est obsolète : IPv4 seulement, et non réentrant. `getaddrinfo` le remplace.
- Après `recvfrom`, vérifier que le nombre d'octets reçus vaut `sizeof(ntp_packet)` avant de lire
  les champs, et que la réponse vient bien de l'adresse interrogée.
- Comparer l'*originate timestamp* de la réponse au *transmit timestamp* envoyé (RFC 5905) :
  une réponse qui ne correspond pas peut être usurpée.
- Fixer un délai de réception (`SO_RCVTIMEO`) pour ne pas bloquer indéfiniment si le serveur ne répond pas.
- NTP n'est pas authentifié : en production, utiliser NTS (RFC 8915).

## Version durcie

`src/ntpclient-udp-secure.c` applique ces points :

| Vérification | Ce qu'elle empêche |
|---|---|
| `getaddrinfo` au lieu de `gethostbyname` | limitation à IPv4, fonction non réentrante |
| `SO_RCVTIMEO` (5 s) | blocage infini si le serveur ne répond pas |
| transmit timestamp avec fraction aléatoire (`getrandom`), puis comparaison à l'*originate timestamp* reçu | réponse forgée par un attaquant qui ne voit pas la requête |
| adresse et port de l'expéditeur comparés au serveur interrogé (`recvfrom`) | datagramme envoyé par une autre machine |
| taille reçue = 48 octets | lecture de champs absents |
| mode = 4, version 3 ou 4, stratum ≠ 0, LI ≠ 3 | réponse qui n'en est pas une, « Kiss-o'-Death », horloge non synchronisée |

Ces vérifications ne remplacent pas une authentification : un attaquant placé sur le chemin
voit la requête et peut forger une réponse valide. Seul NTS protège contre lui.

Test local avec un faux serveur qui rejoue chaque attaque :

```sh
python3 tests/faux_serveur_ntp.py usurpation 12345 &
./bin/ntpclient-udp-secure 127.0.0.1 12345
# Réponse rejetée : l'originate timestamp ne correspond pas à la requête envoyée
```

Scénarios : `ok`, `usurpation`, `court`, `kod`, `non-synchro`, `intrus`, `muet`.
