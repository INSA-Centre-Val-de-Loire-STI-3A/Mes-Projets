<div align="center">

# INSA CVL — Sécurité et Technologies Informatiques

**Programmation système et réseau en C sous Linux · Administration réseau sur Proxmox**

[![build](https://github.com/Tienou4/insa-cvl-sti/actions/workflows/build.yml/badge.svg)](https://github.com/Tienou4/insa-cvl-sti/actions/workflows/build.yml)
![C](https://img.shields.io/badge/C-GNU11-00599C?logo=c&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-POSIX-FCC624?logo=linux&logoColor=black)
![Proxmox](https://img.shields.io/badge/Proxmox-Debian%2013-E57000?logo=proxmox&logoColor=white)

</div>

Travaux de 3e année du cycle ingénieur **STI** (formation labellisée SecNumEdu par l'ANSSI)
à l'INSA Centre Val de Loire. Le dépôt suit le semestre : chaque TD et TP a déjà son
dossier, et se remplit au fur et à mesure.

## Compétences travaillées

| Domaine | Notions |
|---|---|
| **Programmation système** | appels système bruts (`syscall`), `errno`, gestion des erreurs, fichiers et inodes, verrous, processus, `exec`, redirections |
| **Programmation réseau** | sockets UDP et TCP, protocole NTP, résolution DNS, ordre des octets réseau, serveur multi-client (`poll`), protocole texte type IRC |
| **Administration réseau** | routage statique, IP forwarding, NAT / masquerade (nftables), DHCP (isc-dhcp-server), DNS (bind9), rebond SSH |

## Avancement

| Cours | Travail | Statut |
|---|---|---|
| Programmation système | [TD1 — Appels système](programmation-systeme/td/td01-appels-systeme/) | ✅ Terminé |
| | TD1 bis à TD8 · TP mini-glibc (`mini_cat`, mini-shell) | ⏳ À venir |
| Programmation réseau | [TD1 — Client NTP en UDP](programmation-reseaux/td/td01-client-ntp/) | 🚧 En cours |
| | TD2 UDP / TCP · TD3 serveur multi-client · TP noté IRC | ⏳ À venir |
| Réseaux | [TP noté — Réseau de conteneurs sur Proxmox](reseaux/tp/tp01-proxmox-note/) | 🚧 En cours |

## Organisation

```
programmation-systeme/    C / POSIX : TD (appels système → redirections) et TP mini-glibc
programmation-reseaux/    sockets en C : TD (NTP, UDP/TCP, multi-client) et TP noté IRC
reseaux/                  TP Proxmox : configs rangées par machine, scripts de test, rapport
```

Chaque TD suit le même schéma : `README.md` (énoncé résumé, exercices),
`Makefile`, `src/` (un exécutable par fichier `.c`).

## Compiler et lancer

Prérequis : Linux, `gcc`, `make`.

```sh
git clone https://github.com/Tienou4/insa-cvl-sti.git
cd insa-cvl-sti/programmation-systeme/td/td01-appels-systeme
make
./bin/ex02_03_erreur      # write: Bad file descriptor
```

À chaque push, GitHub Actions compile toutes les sources C (`-Wall -Wextra -Werror`, analyseur
statique de gcc), les passe à `cppcheck` et exécute les TD système sous AddressSanitizer.

## Auteur

**Ulrich Brayanne Tienou** — élève-ingénieur en cybersécurité, INSA CVL.
Recherche une **alternance de 2 ans à partir de septembre 2027**.

[![LinkedIn](https://img.shields.io/badge/LinkedIn-ulrich--tienou-0A66C2?logo=linkedin&logoColor=white)](https://www.linkedin.com/in/ulrich-tienou)
[![GitHub](https://img.shields.io/badge/GitHub-Tienou4-181717?logo=github)](https://github.com/Tienou4)

Autres projets : [c-data-structures](https://github.com/Tienou4/c-data-structures) ·
[matrix-logic-puzzles](https://github.com/Tienou4/matrix-logic-puzzles)
