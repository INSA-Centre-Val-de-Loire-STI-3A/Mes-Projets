# TP noté — Proxmox (3A STI, septembre 2026)

Mise en place d'un petit réseau de conteneurs Debian 13 derrière un OPNsense
fourni : routage statique, IP forwarding, masquerade, DHCP, DNS, NAT
one-to-one et rebond SSH.

## Topologie

```
 client2                 client1 (box)                client3
 10.10.2.2  ── vmbr2 ──  10.10.2.254                   10.10.3.2
                         10.10.3.254   ── vmbr3 ──────┘
                         DHCP (vmbr1)
                              │
                            vmbr1  10.0.0.0/24
                              │
                       OPNsense (fourni) ── vmbr0 ── Internet
```

## Plan d'adressage

| Machine | Hostname | CT ID | Interface | Bridge | Adresse |
|---|---|---|---|---|---|
| client1 | `tienoubobcat1` | 1101 | eth0 | vmbr1 | DHCP (+ IP statique secondaire, Q6.7) |
| | | | eth1 | vmbr2 | 10.10.2.254/24 |
| | | | eth2 | vmbr3 | 10.10.3.254/24 |
| client2 | `tienoubobcat2` | 1102 | eth0 | vmbr2 | 10.10.2.2/24, gw 10.10.2.254 |
| client3 | `tienoubobcat3` | 1103 | eth0 | vmbr3 | 10.10.3.2/24, gw 10.10.3.254 |
| cloneTest | `cloneTest` | 1110 | eth0 | vmbr3 puis vmbr1 | DHCP |

Domaine DNS : `tienou-tp.insa-cvl.fr` (serveur bind9 sur client2).

## Organisation du dossier

```
configs/      fichiers de configuration, rangés selon leur chemin sur chaque machine
scripts/      commandes de test rejouables
captures/     captures d'écran utilisées dans le rapport
rapport.md    réponses aux questions + captures
```

## Avancement

- [ ] Ex. 1 — Adresses statiques et routage IP
- [ ] Ex. 2 — Forwarding et masquerade
- [ ] Ex. 3 — Serveur DHCP
- [ ] Ex. 4 — Serveur DNS
- [ ] Ex. 5 — NAT one-to-one
- [ ] Ex. 6 — SSH
