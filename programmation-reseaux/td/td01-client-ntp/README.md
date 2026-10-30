# TD1 — Socket et client NTP

Interroger un serveur NTP (`fr.pool.ntp.org`, UDP port 123) et afficher l'heure reçue.

`src/ntpclient-udp-sendto.c` contient :

- la structure `ntp_packet` (48 octets, RFC 5905) et les macros `SET_LI` / `SET_VN` / `SET_MODE`
  qui remplissent l'octet LI / version / mode ;
- la création de la socket UDP et la résolution du serveur avec `gethostbyname`.

En cours : l'envoi de la requête (`sendto`), la réception de la réponse (`recvfrom`),
puis la conversion du timestamp NTP en heure locale (`ntohl`, décalage de 70 ans, `ctime`).

```sh
make
./bin/ntpclient-udp-sendto
```

Vérification avec Wireshark : filtre `udp.port == 123`.
