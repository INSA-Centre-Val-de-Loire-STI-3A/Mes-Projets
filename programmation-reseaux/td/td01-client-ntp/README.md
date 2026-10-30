# TD1 — Socket et client NTP

Interroger un serveur NTP (`fr.pool.ntp.org`, UDP port 123) et afficher l'heure reçue.

| Fichier | Exercices |
|---|---|
| `src/ntpclient-udp.c` | 1-24 : `error()`, struct `ntp_packet` (48 octets), macros `SET_LI/VN/MODE`, `socket`, `gethostbyname`, `connect`, `write`/`read`, `ntohl`, `NTP_TIMESTAMP_DELTA`, `ctime` |
| `src/ntpclient-udp-sendto.c` | 25-26 : serveur passé en argument, `sendto`/`recvfrom` à la place de `connect` + `read`/`write` |

```sh
make
./bin/ntpclient-udp
./bin/ntpclient-udp-sendto fr.pool.ntp.org
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
