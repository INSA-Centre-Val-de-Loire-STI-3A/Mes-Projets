#!/usr/bin/env bash
# Tests du serveur et du client IRC. Les clients de test parlent le protocole
# directement via /dev/tcp (bash), le dernier test utilise le vrai client.
set -u
cd "$(dirname "$0")/.."
ROOT=$PWD
TMP=$(mktemp -d)
PORT=$(( 20000 + RANDOM % 20000 ))
trap 'kill $SRV $CLI 2>/dev/null; exec 7>&- 2>/dev/null; rm -rf "$TMP"' EXIT
SRV= CLI=
pass=0

stdbuf -oL ./bin/irc_server $PORT "$TMP/accounts.txt" > "$TMP/server.log" 2>&1 & SRV=$!
sleep 0.3

fail() { echo "ÉCHEC : $*"; echo "--- journal du serveur"; cat "$TMP/server.log"; exit 1; }
ok()   { pass=$((pass + 1)); echo "  ok  $*"; }

# connect VAR : ouvre une connexion, le descripteur est rangé dans VAR
connect() { exec {fd}<>"/dev/tcp/127.0.0.1/$PORT" || fail "connexion"; printf -v "$1" %s "$fd"; }
say()     { printf '%s\n' "$2" >&"$1"; }
# expect FD MOTIF : lit des lignes jusqu'à en trouver une qui contient MOTIF
expect() {
    local line
    while IFS= read -r -t 2 line <&"$1"; do
        [[ $line == *"$2"* ]] && { LAST=$line; return 0; }
    done
    fail "attendu « $2 » sur le descripteur $1"
}
# nothing FD : aucune ligne ne doit arriver
nothing() { local line; if IFS= read -r -t 0.3 line <&"$1"; then fail "ligne inattendue : $line"; fi; }
# drain FD... : jette les lignes déjà reçues (annonces sans intérêt pour le test suivant)
drain()   { local fd line; for fd; do while IFS= read -r -t 0.2 line <&"$fd"; do :; done; done; }

echo "Serveur sur le port $PORT"

# --- ex. 18-19 : pseudos uniques
connect A; expect $A "Bienvenue"
say $A "/nickname alice";  expect $A "Vous êtes maintenant alice"
connect B; expect $B "Bienvenue"
say $B "bonjour";          expect $B "Choisissez d'abord un pseudo"
say $B "/nickname alice";  expect $B "déjà utilisé"
say $B "/nickname b@d";    expect $B "pseudo invalide"
say $B "/nickname bob";    expect $B "Vous êtes maintenant bob"
expect $A "bob a rejoint"
ok "pseudos uniques et valides"

# --- ex. 17 : diffusion aux autres clients seulement
say $B "bonjour à tous";   expect $A "bob : bonjour à tous"; nothing $B
ok "diffusion"

# --- ex. 20 : /date
say $A "/date";            expect $A "Heure du serveur"
ok "/date"

# --- ex. 21 : message privé
connect C; expect $C "Bienvenue"; say $C "/nickname carol"; expect $C "Vous êtes maintenant carol"
expect $A "carol a rejoint"; expect $B "carol a rejoint"
say $A "/mp bob c'est secret"; expect $B "private message from : alice : c'est secret"; nothing $C
say $A "/mp personne x";       expect $A "n'est pas connecté"
ok "/mp"

# --- ex. 22 : alertes (bip \a + gras rouge)
say $A "/alerte bob réveille-toi"; expect $B "[ALERTE de alice] réveille-toi"; nothing $C
[[ $LAST == $'\a\e[1;31m'* ]] || fail "l'alerte n'a ni bip ni couleur"
say $A "/alerte tout le monde";    expect $B "[ALERTE de alice] tout le monde"; expect $C "[ALERTE de alice] tout le monde"
ok "/alerte privée et générale"

# --- ex. 24.1 : couleurs ; et pas d'injection de séquences ANSI par un utilisateur
say $B "/green vert";              expect $A "bob : "$'\e[32m'"vert"
say $B $'piège \e[2J\a';           expect $A "bob : piège ?[2J?"
ok "couleurs, séquences de contrôle neutralisées"

# --- ex. 19 : /register, /nickname pseudo mdp, /unregister
say $A "/register alice motdepasse";  expect $A "Pseudo alice enregistré"
say $A "/register alice autre";       expect $A "déjà enregistré"
grep -q 'motdepasse' "$TMP/accounts.txt" && fail "mot de passe stocké en clair"
grep -q '^alice \$' "$TMP/accounts.txt" || fail "hachage absent du fichier des comptes"
[ "$(stat -c %a "$TMP/accounts.txt")" = 600 ] || fail "fichier des comptes lisible par tous"
# carol occupe « dave », bob l'enregistre puis le réclame : carol doit changer de pseudo
say $C "/nickname dave";              expect $C "Vous êtes maintenant dave"
say $B "/register dave secret123";    expect $B "Pseudo dave enregistré"
say $B "/nickname dave mauvais";      expect $B "mot de passe incorrect"
say $B "/nickname dave";              expect $B "utilisez /nickname dave mot-de-passe"
say $B "/nickname dave secret123";    expect $C "réclamé par son propriétaire"; expect $C "Vous êtes maintenant invite"
expect $B "Vous êtes maintenant dave (pseudo enregistré)"
say $C "/who"; expect $C "Connectés"
say $A "/unregister dave faux";       expect $A "mot de passe incorrect"
say $A "/unregister dave secret123";  expect $A "Pseudo dave supprimé"
ok "/register, réclamation d'un pseudo, /unregister"

# --- ex. 24.2-3 : salons et modération
say $B "/join secret";     expect $B "Salon #secret créé"
say $C "/join secret";     expect $C "Vous avez rejoint #secret"; expect $B "a rejoint #secret"
drain $A
say $C "coucou";           expect $B "[#secret] invite"; nothing $A
say $C "/kick dave";       expect $C "pas modérateur"
say $B "/who";             expect $B "(modérateur)"
INV=$(grep -o 'invite[0-9]*' "$TMP/server.log" | tail -1)
say $B "/kick $INV";       expect $C "expulsé de #secret par dave"; expect $C "Vous avez rejoint #general"
ok "salons, modérateur, /kick"

# --- ex. 23 : envoi de fichier (protocole brut)
printf '/send dave note.txt 12\nhello\nworld\n' >&$C
expect $B "/file $INV note.txt 12"
IFS= read -r -N 12 -t 2 data <&$B; [ "$data" = $'hello\nworld\n' ] || fail "contenu du fichier : « $data »"
expect $C "Fichier transmis"
printf '/send dave ../evil 3\nabc' >&$C; expect $C "nom de fichier refusé"; expect $C "Fichier ignoré"
say $C "/date"; expect $C "Heure du serveur"     # le flux est resté synchronisé
drain $B
ok "/send relayé, nom de fichier dangereux refusé"

# --- vrai client : choix du pseudo, horodatage, réception d'un fichier
mkdir "$TMP/cli" && mkfifo "$TMP/cli/in"
( cd "$TMP/cli" && exec "$ROOT/bin/irc_client" 127.0.0.1 $PORT none < in > out 2>&1 ) & CLI=$!
exec 7> "$TMP/cli/in"
printf 'alice\nfrank\n' >&7                       # alice est enregistré : refusé, puis frank
sleep 0.5
grep -q "Vous êtes maintenant frank" "$TMP/cli/out" || fail "le client n'a pas obtenu son pseudo : $(cat "$TMP/cli/out")"
grep -qE '^\[[0-9]{2}:[0-9]{2}:[0-9]{2}\] \*\*\* Erreur' "$TMP/cli/out" || fail "messages non horodatés"
echo "contenu du fichier" > "$TMP/envoi.txt"
say $A "/nickname alice2"; expect $A "Vous êtes maintenant alice2"
printf '/send frank envoi.txt 19\ncontenu du fichier\n' >&$A
sleep 0.5
cmp -s "$TMP/envoi.txt" "$TMP/cli/envoi.txt" || fail "fichier non reçu par le client : $(cat "$TMP/cli/out")"
printf '/send frank envoi.txt 19\ncontenu du fichier\n' >&$A
sleep 0.5
[ -f "$TMP/cli/envoi.txt.1" ] || fail "le second fichier a écrasé le premier"
echo "/send alice2 $TMP/envoi.txt" >&7
expect $A "/file frank envoi.txt 19"
exec 7>&-                                          # Ctrl+D : le client envoie /exit
wait $CLI; CLI=
grep -q "Au revoir" "$TMP/cli/out" || fail "le client n'a pas quitté proprement"
ok "client : pseudo, horodatage, fichiers reçus sans écrasement, /send, sortie"

# --- ex. 20 : /exit
say $A "/exit"; expect $A "Au revoir"; expect $C "alice2 a quitté le chat"   # C est revenu dans #general
ok "/exit"

echo "test_irc : OK ($pass groupes de tests)"
