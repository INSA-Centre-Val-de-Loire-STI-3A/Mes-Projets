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
