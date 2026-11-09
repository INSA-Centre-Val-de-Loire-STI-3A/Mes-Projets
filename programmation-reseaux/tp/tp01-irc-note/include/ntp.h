/* Heure de réception des messages via NTP (code du TD1).
   Une seule requête NTP au lancement : on mémorise l'écart entre l'heure NTP
   et l'horloge locale, puis l'heure de chaque message = horloge locale + écart.
   (Une requête par message serait lente et ferait bannir le client par le
   pool NTP, qui limite le nombre de requêtes.) */
#ifndef NTP_H
#define NTP_H

#include <time.h>

/* interroge le serveur NTP ; 0 si l'heure est synchronisée, -1 sinon
   (l'horloge locale est alors utilisée) */
int    ntp_init(const char *server);
time_t ntp_now(void);

#endif
