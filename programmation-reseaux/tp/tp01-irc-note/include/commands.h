/* Traitement d'une ligne reçue d'un client (message ou commande). */
#ifndef COMMANDS_H
#define COMMANDS_H

#include "clients.h"

void handle_line(struct client *c, char *line);

#endif
