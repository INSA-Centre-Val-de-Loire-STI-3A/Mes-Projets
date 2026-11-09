# 1. Sockets et UDP — client NTP

## La socket

Une socket est un descripteur de fichier qui représente une extrémité de communication.

```c
int fd = socket(AF_INET, SOCK_DGRAM, 0);   // IPv4 + UDP   (SOCK_STREAM = TCP)
```

| Famille | Adresse | Structure |
|---|---|---|
| `AF_INET` | IPv4 | `struct sockaddr_in` (`sin_family`, `sin_port`, `sin_addr`) |
| `AF_INET6` | IPv6 | `struct sockaddr_in6` |

Les fonctions prennent un `struct sockaddr *` générique : on caste l'adresse concrète.

## Ordre des octets

Le réseau est en **big-endian**, x86 en little-endian. On convertit tout champ de plus d'un octet :
`htons` / `htonl` (hôte → réseau) avant l'envoi, `ntohs` / `ntohl` (réseau → hôte) à la réception.
Oublier `htons(port)` fait écouter sur un autre port (1234 devient 53764).

## Résoudre un nom

- `gethostbyname` : ancien, IPv4 seulement, non réentrant (résultat dans une zone statique).
- `getaddrinfo(host, service, &hints, &res)` : IPv4 et IPv6, réentrant, renvoie une liste ;
  libérer avec `freeaddrinfo`. Erreur lisible avec `gai_strerror`.
- `inet_pton` / `inet_ntop` : texte ↔ binaire pour une adresse IP déjà connue.

## UDP

Pas de connexion, des **datagrammes** : chaque `sendto` produit un datagramme, reçu entier par un `recvfrom`
(ou perdu). Pas de garantie d'arrivée, d'ordre ni d'unicité.

```c
sendto(fd, buf, len, 0, (struct sockaddr *)&dest, sizeof(dest));
recvfrom(fd, buf, size, 0, (struct sockaddr *)&from, &fromlen);  // from = qui a envoyé
```

`connect` sur une socket UDP fixe le destinataire : on peut alors utiliser `read`/`write`, et le noyau
ne remonte plus que les datagrammes de cette adresse.

## NTP (RFC 5905)

Requête et réponse de 48 octets sur UDP 123. Le client met `LI = 0, VN = 4, Mode = 3` ; le serveur
répond en mode 4. Les horodatages comptent les secondes depuis **1900** : soustraire
`2208988800` pour obtenir l'heure Unix (depuis 1970).

## Sécurité

UDP n'authentifie pas l'expéditeur : n'importe qui peut envoyer une fausse réponse. Défenses (TD1,
version durcie) : vérifier l'adresse de l'expéditeur, la taille, et que l'*originate timestamp* de la
réponse recopie le *transmit timestamp* aléatoire de la requête ; poser un délai (`SO_RCVTIMEO`).
Contre un attaquant sur le chemin, seule une authentification cryptographique (NTS, RFC 8915) suffit.
