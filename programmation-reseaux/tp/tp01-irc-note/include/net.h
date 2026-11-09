/* Outils réseau communs au client et au serveur IRC. */
#ifndef NET_H
#define NET_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define LINEBUF_SIZE 8192   /* tampon de réception d'une socket */

/* Octets reçus mais pas encore traités. TCP est un flux : un recv() peut
   contenir une demi-ligne ou plusieurs lignes, on accumule ici. */
struct linebuf {
    char   data[LINEBUF_SIZE];
    size_t len;
};

void     stop(const char *msg);                    /* perror + exit */
uint16_t parse_port(const char *s);                /* 1-65535, sinon exit */
int      tcp_listen(uint16_t port);                /* socket + bind + listen, toutes interfaces */
int      tcp_connect(const char *host, uint16_t port);

int send_all(int fd, const void *buf, size_t len); /* 0, ou -1 si l'envoi échoue */
int sendf(int fd, const char *fmt, ...)            /* une ligne formatée, '\n' ajouté */
    __attribute__((format(printf, 2, 3)));

/* recv() à la suite des données déjà présentes : >0 octets lus, 0 fin de flux,
   -1 erreur, -2 tampon plein (ligne trop longue) */
ssize_t linebuf_fill(int fd, struct linebuf *lb);
/* extrait une ligne complète sans '\n' ni '\r' (tronquée à size-1) : 1 si trouvée */
int     linebuf_getline(struct linebuf *lb, char *line, size_t size);
/* extrait au plus max octets bruts (contenu d'un fichier) : nombre d'octets copiés */
size_t  linebuf_take(struct linebuf *lb, void *out, size_t max);

#endif
