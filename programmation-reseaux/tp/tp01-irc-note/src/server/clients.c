/* Table des clients connectés, unicité des pseudos, diffusion. */

#include "clients.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include "accounts.h"
#include "channels.h"

struct client clients[MAX_CLIENTS];

void clients_init(void)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
        clients[i].fd = -1;
}

struct client *client_add(int fd, const char *addr)
{
    for (int i = 0; i < MAX_CLIENTS; i++) {
        struct client *c = &clients[i];
        if (c->fd != -1) continue;
        memset(c, 0, sizeof(*c));
        c->fd = fd;
        c->file_dest = -1;
        snprintf(c->addr, sizeof(c->addr), "%s", addr);
        snprintf(c->channel, sizeof(c->channel), "%s", DEFAULT_CHANNEL);
        return c;
    }
    return NULL;
}

void client_remove(struct client *c)
{
    if (c->nick[0] != '\0') {
        printf("%s (%s) déconnecté\n", c->nick, c->addr);
        channels_leave(c);
        broadcast(c->channel, c, "*** %s a quitté le chat", c->nick);
    } else {
        printf("Client %s déconnecté\n", c->addr);
    }
    /* un fichier était en cours d'envoi vers ce client : on jettera la suite */
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (clients[i].fd != -1 && clients[i].file_dest == c->fd)
            clients[i].file_dest = -1;
    close(c->fd);
    c->fd = -1;
}

struct client *client_by_nick(const char *nick)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (clients[i].fd != -1 && !clients[i].closing && strcmp(clients[i].nick, nick) == 0)
            return &clients[i];
    return NULL;
}

struct client *client_by_fd(int fd)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (clients[i].fd == fd)
            return &clients[i];
    return NULL;
}

static void vsend(struct client *c, const char *fmt, va_list ap)
{
    char buf[MSG_MAX + 256];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    if (sendf(c->fd, "%s", buf) == -1)
        c->closing = 1;
}

void client_send(struct client *c, const char *fmt, ...)
{
    if (c->closing) return;
    va_list ap;
    va_start(ap, fmt);
    vsend(c, fmt, ap);
    va_end(ap);
}

void broadcast(const char *channel, const struct client *except, const char *fmt, ...)
{
    for (int i = 0; i < MAX_CLIENTS; i++) {
        struct client *c = &clients[i];
        if (c->fd == -1 || c->closing || c == except || c->nick[0] == '\0'
            || strcmp(c->channel, channel) != 0)
            continue;
        va_list ap;
        va_start(ap, fmt);
        vsend(c, fmt, ap);
        va_end(ap);
    }
}

void client_set_nick(struct client *c, const char *nick, int authenticated)
{
    char old[NICK_MAX + 1];
    snprintf(old, sizeof(old), "%s", c->nick);
    snprintf(c->nick, sizeof(c->nick), "%s", nick);
    c->authenticated = authenticated;

    if (old[0] == '\0') {
        printf("%s choisit le pseudo %s\n", c->addr, nick);
        broadcast(c->channel, c, "*** %s a rejoint #%s", nick, c->channel);
    } else {
        printf("%s devient %s\n", old, nick);
        broadcast(c->channel, c, "*** %s s'appelle maintenant %s", old, nick);
    }
    client_send(c, "*** Vous êtes maintenant %s%s", nick,
                authenticated ? " (pseudo enregistré)" : "");
}

void guest_nick(char *out, size_t size)
{
    static unsigned counter = 0;
    do {
        snprintf(out, size, "invite%u", ++counter);
    } while (client_by_nick(out) != NULL || accounts_exists(out));
}
