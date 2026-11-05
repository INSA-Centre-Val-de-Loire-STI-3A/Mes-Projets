"""Faux serveur NTP pour tester ntpclient-udp-secure en local.

Usage : python3 tests/faux_serveur_ntp.py <scenario> [port]
puis, dans un autre terminal : ./bin/ntpclient-udp-secure 127.0.0.1 <port>

Scénarios :
  ok           réponse correcte                       -> heure acceptée
  usurpation   originate timestamp faux               -> rejet
  court        réponse tronquée (20 octets)           -> rejet
  kod          stratum 0 (« Kiss-o'-Death »)          -> rejet
  non-synchro  LI = 3                                 -> rejet
  intrus       un datagramme part d'un autre port,
               puis la vraie réponse                  -> intrus ignoré, heure acceptée
  muet         aucune réponse                         -> délai dépassé
"""

import socket
import struct
import sys
import time

NTP_DELTA = 2208988800
SCENARIOS = ("ok", "usurpation", "court", "kod", "non-synchro", "intrus", "muet")


def reponse(requete: bytes, li: int = 0, stratum: int = 2, orig: bytes | None = None) -> bytes:
    """Construit une réponse serveur (mode 4) à partir de la requête du client."""
    maintenant = int(time.time()) + NTP_DELTA
    li_vn_mode = (li << 6) | (4 << 3) | 4
    entete = struct.pack("!BBbb3I", li_vn_mode, stratum, 6, -20, 0, 0, 0x4C4F434C)  # refId "LOCL"
    ref = struct.pack("!II", maintenant, 0)
    origine = requete[40:48] if orig is None else orig  # recopie du transmit timestamp client
    rx_tx = struct.pack("!IIII", maintenant, 0, maintenant, 0)
    return entete + ref + origine + rx_tx


def main() -> None:
    scenario = sys.argv[1] if len(sys.argv) > 1 else "ok"
    port = int(sys.argv[2]) if len(sys.argv) > 2 else 12345
    if scenario not in SCENARIOS:
        sys.exit(f"scénario inconnu : {scenario} (au choix : {', '.join(SCENARIOS)})")

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as srv:
        srv.bind(("127.0.0.1", port))
        print(f"faux serveur NTP sur 127.0.0.1:{port}, scénario « {scenario} »", flush=True)
        requete, client = srv.recvfrom(1024)
        assert len(requete) == 48, "requête NTP de taille inattendue"

        if scenario == "ok":
            srv.sendto(reponse(requete), client)
        elif scenario == "usurpation":
            srv.sendto(reponse(requete, orig=b"\x00" * 8), client)
        elif scenario == "court":
            srv.sendto(reponse(requete)[:20], client)
        elif scenario == "kod":
            srv.sendto(reponse(requete, stratum=0), client)
        elif scenario == "non-synchro":
            srv.sendto(reponse(requete, li=3), client)
        elif scenario == "intrus":
            with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as autre:
                autre.sendto(reponse(requete, orig=b"\x00" * 8), client)
            time.sleep(0.1)
            srv.sendto(reponse(requete), client)
        # "muet" : on ne répond rien


if __name__ == "__main__":
    main()
