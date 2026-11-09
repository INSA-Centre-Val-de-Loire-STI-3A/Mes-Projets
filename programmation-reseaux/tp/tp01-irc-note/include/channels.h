/* Salons : /join crée le salon s'il n'existe pas, son créateur en est le
   modérateur et peut utiliser /kick. Le salon par défaut n'a pas de modérateur. */
#ifndef CHANNELS_H
#define CHANNELS_H

#include "clients.h"

void channels_join(struct client *c, const char *name);
void channels_leave(struct client *c);          /* avant de quitter le salon courant */
int  channels_is_moderator(const char *name, const struct client *c);

#endif
