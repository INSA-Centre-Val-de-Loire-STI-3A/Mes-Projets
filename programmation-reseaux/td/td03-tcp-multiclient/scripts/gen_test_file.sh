#!/bin/sh
# TD3, ex. 1 : fichier de test (100000 lignes « coucou » puis « end »).
# Version de l'énoncé : for i in `seq 1 100000`; do echo coucou >> test.txt; done
# (100000 ouvertures du fichier). Même résultat en une seule écriture :
set -e
out="${1:-test.txt}"
{ yes coucou | head -n 100000; echo end; } > "$out"
echo "$out : $(wc -l < "$out") lignes, $(wc -c < "$out") octets"
