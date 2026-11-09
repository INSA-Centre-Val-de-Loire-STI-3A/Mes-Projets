/* Protocole texte du chat : une ligne terminée par '\n' par message.

   client → serveur : « texte » (message au salon) ou « /commande arguments »
                      /send <pseudo> <nom> <taille>\n suivi de <taille> octets
   serveur → client : lignes de texte (« *** » = message du serveur),
                      /file <expéditeur> <nom> <taille>\n suivi de <taille> octets */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>

#define MSG_MAX       1024              /* longueur max d'une ligne */
#define NICK_MAX      20
#define CHANNEL_MAX   20
#define PASSWORD_MAX  64
#define FILENAME_MAX_LEN 64
#define FILE_MAX      (10 * 1024 * 1024) /* taille max d'un fichier envoyé */
#define DEFAULT_CHANNEL "general"

/* mot suivant (séparé par des espaces) ; NULL s'il n'y en a plus.
   La chaîne est modifiée (un '\0' termine le mot) et *s avance. */
char *next_word(char **s);
/* reste de la ligne, sans les espaces de tête (peut être "") */
char *rest_of_line(char **s);

int valid_nick(const char *s);      /* 1..NICK_MAX caractères [A-Za-z0-9_-] */
int valid_channel(const char *s);   /* idem, CHANNEL_MAX */
int valid_filename(const char *s);  /* [A-Za-z0-9._-], pas de '/', ne commence pas par '.' */
int parse_size(const char *s, size_t *size); /* entier décimal 0..FILE_MAX : 1 si valide */

/* remplace les caractères de contrôle (ESC, \a, \r...) par '?' : un
   utilisateur ne doit pas pouvoir envoyer de séquences ANSI aux autres */
void sanitize(char *s);

/* séquence ANSI d'une couleur (« red », « blue »...) ou NULL */
const char *ansi_color(const char *name);

#define ANSI_RESET "\033[0m"
#define ANSI_ALERT "\033[1;31m"         /* gras + rouge */

#endif
