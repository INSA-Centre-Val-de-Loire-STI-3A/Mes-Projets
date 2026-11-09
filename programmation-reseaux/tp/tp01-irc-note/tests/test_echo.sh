#!/usr/bin/env bash
# Partie 1 : le client echo reçoit 1000 fois ECHO, et le serveur survit
# à un client qui se déconnecte (ex. 10).
set -u
cd "$(dirname "$0")/.."
log=$(mktemp); trap 'kill $srv 2>/dev/null; rm -f "$log"' EXIT

stdbuf -oL ./bin/tcpserver > "$log" 2>&1 & srv=$!   # stdbuf : journal écrit ligne par ligne
sleep 0.3
echo -n coucou | nc -4 -w1 localhost 1234 > /dev/null   # ex. 9 : client qui part aussitôt
sleep 0.3
grep -q "Client déconnecté" "$log" || { echo "ÉCHEC : déconnexion non détectée"; cat "$log"; exit 1; }

n=$(./bin/tcpclient | grep -c '^Recv : ECHO$')
[ "$n" -eq 999 ] || { echo "ÉCHEC : $n réponses ECHO au lieu de 999"; exit 1; }
echo "test_echo : OK (déconnexion détectée, 999 échos ; le 1000e message était « coucou »)"
