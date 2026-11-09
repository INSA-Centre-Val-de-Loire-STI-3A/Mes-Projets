# TP noté IRC — rapport

Serveur de chat TCP multi-client et son client, en C, sans thread ni `fork` : chaque programme
est un seul processus qui attend les événements avec `poll()`.

## Partie 1 — echo TCP (ex. 1-16)

`src/echo/tcpserver.c` et `src/echo/tcpclient.c`, repris du TD2.

**Ex. 10 — que se passe-t-il avec `echo -n coucou | nc -4 -w0 localhost 1234` ?**
`nc` envoie `coucou` et ferme aussitôt la connexion. Le serveur affiche et renvoie `coucou`, puis
chaque `recv` suivant renvoie **0** (fin de flux). `0` n'étant pas une erreur, le serveur d'origine
continue sa boucle : il affiche 999 messages vides et ses `send` vers la connexion fermée finissent
par lever `SIGPIPE`, qui tue le processus.

**Correction :** tester `recv() == 0` (client déconnecté) pour fermer sa socket et attendre un autre
client, et envoyer avec `MSG_NOSIGNAL` pour obtenir l'erreur `EPIPE` au lieu du signal.
`SO_REUSEADDR` permet de relancer le serveur sans attendre la fin du `TIME_WAIT`.

## Partie 2 — serveur multi-client (ex. 17)

`src/server/main.c` : à chaque tour, un tableau `pollfd` contient la socket d'écoute et toutes les
sockets clientes.

- socket d'écoute prête → `accept`, le client est ajouté à la table (`clients.c`, 64 places) ;
- socket cliente prête → `recv` dans le **tampon de lignes** du client, puis traitement de chaque
  ligne complète ; `0` ou erreur → déconnexion.

Deux points de conception :

- **TCP est un flux.** Un `recv` peut contenir une demi-ligne ou plusieurs lignes. Chaque client a donc
  un tampon (`struct linebuf`) où les octets s'accumulent ; on n'en extrait que des lignes terminées
  par `\n`.
- **Fermeture différée.** Un client à supprimer (déconnexion, `/exit`, envoi impossible) est seulement
  marqué `closing` ; il est fermé à la fin du tour de `poll`. Aucun pointeur vers un client supprimé
  n'est donc utilisé pendant qu'on traite les autres.

## Partie 3 — client (ex. 18)

`src/client/main.c` : `./irc_client ip port [serveur-ntp]`.

1. IP (ou nom) et port en arguments (`getaddrinfo`).
2. Le pseudo est demandé au lancement et envoyé par `/nickname` ; tant que le serveur répond
   `*** Erreur`, le client redemande. On peut taper `pseudo mot-de-passe` pour un pseudo enregistré.
3. **Heure NTP** (`ntp.c`, code durci du TD1). Une seule requête au lancement donne l'écart entre
   l'heure NTP et l'horloge locale ; chaque message reçu est horodaté avec l'horloge locale corrigée
   de cet écart. Une requête par message ajouterait jusqu'à un aller-retour réseau par ligne et ferait
   bannir le client par le pool NTP, qui limite le débit des requêtes. Si NTP ne répond pas en 2 s,
   l'horloge locale est utilisée (et signalée).

Le client surveille avec `poll()` le clavier et la socket : il affiche les messages reçus pendant
qu'on tape.

## Protocole

Texte, une ligne par message. Les lignes commençant par `/` sont des commandes, les autres sont
diffusées dans le salon, préfixées par le pseudo. Les lignes du serveur commencent par `***`.

L'envoi de fichier mélange texte et binaire :

```
client A → serveur :  /send bob note.txt 12\n  + 12 octets
serveur → client B :  /file alice note.txt 12\n + 12 octets
```

La taille annoncée indique au serveur combien d'octets suivent : il les relaie sans les interpréter,
puis reprend la lecture par lignes. Même si le fichier est refusé (destinataire absent, nom invalide),
le serveur **lit et jette** ces octets, sinon le contenu du fichier serait pris pour des commandes.

## Fonctionnalités (ex. 19-24)

| Ex. | Commande | Implémentation |
|---|---|---|
| 19 | `/nickname pseudo` | refusé si déjà utilisé ou invalide (1-20 caractères `A-Z a-z 0-9 _ -`) |
| 19 | `/register pseudo mdp` | refusé si déjà enregistré ; mot de passe haché, sauvegardé dans `data/accounts.txt` |
| 19 | `/nickname pseudo mdp` | vérifie le mot de passe ; l'utilisateur **non enregistré** qui occupait le pseudo reçoit un pseudo `inviteN` |
| 19 | `/unregister pseudo mdp` | supprime le compte si le mot de passe est bon |
| 20 | `/exit`, `/date` | fermeture propre ; heure du serveur |
| 21 | `/mp pseudo message` | affiché `private message from : expéditeur : message` |
| 22 | `/alerte [pseudo] message` | `\a` (bip) + gras rouge ; si le premier mot est un pseudo connecté, alerte privée, sinon tout le salon |
| 23 | `/send pseudo fichier` | le client lit le fichier, le destinataire l'enregistre dans son répertoire courant |
| 24 | `/red /green /yellow /blue /magenta /cyan` | message coloré (codes ANSI) |
| 24 | `/join salon` | crée le salon s'il n'existe pas ; le créateur est modérateur |
| 24 | `/kick pseudo` | modérateur seulement : renvoie la cible dans `#general` |
| + | `/who`, `/help` | membres du salon, liste des commandes |

Si le modérateur quitte son salon, le rôle passe au membre suivant ; un salon vide disparaît.

## Sécurité

Le serveur reçoit des données de n'importe qui : chaque entrée est validée.

| Risque | Protection |
|---|---|
| Injection de séquences terminal : un utilisateur envoie `\e[2J` (efface l'écran des autres) ou des séquences ANSI plus dangereuses | `sanitize()` remplace tout caractère de contrôle par `?` avant diffusion **et** avant écriture dans le journal du serveur ; seul le serveur ajoute des couleurs |
| Traversée de répertoire : `/send bob ../../.bashrc` | nom de fichier limité à `A-Z a-z 0-9 . _ -`, sans `/` ni `.` initial, vérifié par le serveur **et** par le client récepteur |
| Écrasement d'un fichier du destinataire | le client crée le fichier avec `O_CREAT \| O_EXCL` (jamais d'écrasement, pas de suivi de lien) ; si le nom existe, `nom.1`, `nom.2`… |
| Mots de passe | jamais stockés en clair : `crypt_r` (yescrypt, sel aléatoire, comme `/etc/shadow`), fichier en `0600` écrit atomiquement (`rename`), comparaison en temps constant, mots de passe masqués dans le journal |
| Saturation mémoire | lignes de 1024 caractères max, tampon de 8 Ko par client, fichiers de 10 Mo max, 64 clients |
| Usurpation de l'heure NTP | nonce dans le *transmit timestamp*, `connect` UDP, vérification du mode, du stratum et du LI (TD1) |
| Crash du serveur par un client qui part | `SIGPIPE` ignoré, `MSG_NOSIGNAL` |

**Limites connues :** les échanges ne sont pas chiffrés (mots de passe et messages lisibles par un
attaquant sur le réseau : il faudrait TLS) ; un client qui ne lit plus sa socket peut bloquer le
serveur pendant un `send` (il faudrait des sockets non bloquantes et une file d'envoi par client) ;
aucune limite de débit contre le spam ou l'essai de mots de passe en série.

## Tests

```sh
make test
```

- `tests/test_echo.sh` : le serveur echo détecte la déconnexion de `nc` (ex. 10) puis répond 999 fois
  au client.
- `tests/test_irc.sh` : 11 groupes de tests, avec des clients qui parlent le protocole via `/dev/tcp`
  (bash) et le vrai client pour la fin : pseudos uniques, diffusion, `/date`, `/mp`, alertes,
  couleurs, neutralisation des séquences de contrôle, `/register` (mot de passe absent du fichier,
  droits `600`), réclamation d'un pseudo, `/unregister`, salons, `/kick`, `/send` (relais exact des
  octets, refus de `../evil` sans désynchroniser le flux), client (pseudo refusé puis accepté,
  horodatage, fichiers reçus sans écrasement, `/send`, sortie avec Ctrl+D), `/exit`.

La CI GitHub Actions lance ces tests avec AddressSanitizer et UndefinedBehaviorSanitizer.
