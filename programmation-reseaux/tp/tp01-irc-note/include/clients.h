/* Table des clients connectés au serveur. */
#ifndef CLIENTS_H
#define CLIENTS_H

#include <stddef.h>
#include "net.h"
#include "protocol.h"

#define MAX_CLIENTS 64

struct client {
    int  fd;                          /* -1 : case libre */
    char nick[NICK_MAX + 1];          /* "" tant que le pseudo n'est pas choisi */
    int  authenticated;               /* pseudo enregistré et mot de passe donné */
    char channel[CHANNEL_MAX + 1];
    char addr[32];                    /* ip:port, pour le journal */
    struct linebuf in;
    size_t file_left;                 /* octets d'un /send encore à relayer */
    int    file_dest;                 /* fd du destinataire, -1 : octets jetés */
    int    closing;                   /* à fermer à la fin du tour de poll */
};

extern struct client clients[MAX_CLIENTS];

void           clients_init(void);
struct client *client_add(int fd, const char *addr);   /* NULL si le serveur est plein */
void           client_remove(struct client *c);         /* ferme, prévient le salon */
struct client *client_by_nick(const char *nick);       /* clients nommés seulement */
struct client *client_by_fd(int fd);

/* envoie une ligne ; en cas d'échec le client est marqué à fermer */
void client_send(struct client *c, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
/* envoie une ligne à tous les clients nommés du salon, sauf except (peut être NULL) */
void broadcast(const char *channel, const struct client *except, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));

/* change le pseudo et l'annonce au salon (arrivée si c'est le premier) */
void client_set_nick(struct client *c, const char *nick, int authenticated);
/* pseudo d'invité libre et non enregistré (invite1, invite2...) */
void guest_nick(char *out, size_t size);

#endif
