# INSA CVL — STI

Travaux réalisés en 3e année du cycle ingénieur **Sécurité et Technologies Informatiques**
à l'INSA Centre Val de Loire : programmation système et réseau en C sous Linux,
administration réseau sur Proxmox.

| Dossier | Contenu | Notions |
|---|---|---|
| [`programmation-systeme/`](programmation-systeme/) | TD en C / POSIX | appels système (`syscall`), `errno`, gestion des erreurs |
| [`programmation-reseaux/`](programmation-reseaux/) | TD en C sur les sockets | UDP, protocole NTP, résolution DNS, ordre des octets |
| [`reseaux/`](reseaux/) | TP noté sur Proxmox (conteneurs Debian) | routage, NAT / masquerade, DHCP, DNS bind9, nftables, SSH |

Le dépôt est mis à jour au fil du semestre.

## Compiler

Chaque dossier de C a un `Makefile` (gcc, `-Wall -Wextra`) qui produit un exécutable
par fichier `src/*.c` dans le `bin/` du TD :

```sh
make -C programmation-systeme
make -C programmation-reseaux
```

## Autres projets

- [c-data-structures](https://github.com/Tienou4/c-data-structures)
- [matrix-logic-puzzles](https://github.com/Tienou4/matrix-logic-puzzles)
