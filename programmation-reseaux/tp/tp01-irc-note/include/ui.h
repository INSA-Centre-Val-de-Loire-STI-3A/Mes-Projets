/* Affichage côté client : horodatage, fichiers reçus. */
#ifndef UI_H
#define UI_H

#include <stddef.h>

/* affiche une ligne reçue, précédée de l'heure NTP de réception */
void ui_print(const char *line);
/* message local du client (pas du serveur) */
void ui_info(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
/* crée le fichier reçu dans le répertoire courant sans écraser l'existant ;
   descripteur, ou -1. Le nom réellement utilisé est écrit dans path. */
int  ui_create_file(const char *name, char *path, size_t size);

#endif
