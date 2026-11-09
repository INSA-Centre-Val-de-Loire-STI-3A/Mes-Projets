/* Salons et modérateurs. Un salon existe tant qu'il a au moins un membre. */

#include "channels.h"

#include <string.h>
#include <stdio.h>

#define MAX_CHANNELS 32

struct channel {
    char name[CHANNEL_MAX + 1];
    int  moderator_fd;                 /* -1 : pas de modérateur */
};

static struct channel channels[MAX_CHANNELS];
static int nchannels = 0;

static struct channel *find(const char *name)
{
    for (int i = 0; i < nchannels; i++)
        if (strcmp(channels[i].name, name) == 0)
            return &channels[i];
    return NULL;
}

/* membres du salon, sans compter except */
static struct client *first_member(const char *name, const struct client *except)
{
    for (int i = 0; i < MAX_CLIENTS; i++) {
        struct client *m = &clients[i];
        if (m->fd != -1 && m != except && m->nick[0] != '\0' && !m->closing
            && strcmp(m->channel, name) == 0)
            return m;
    }
    return NULL;
}

void channels_leave(struct client *c)
{
    struct channel *ch = find(c->channel);
    if (ch == NULL) return;            /* salon par défaut */

    struct client *next = first_member(c->channel, c);
    if (next == NULL) {                /* plus personne : le salon disparaît */
        *ch = channels[--nchannels];
        return;
    }
    if (ch->moderator_fd == c->fd) {   /* le modérateur part : on passe la main */
        ch->moderator_fd = next->fd;
        broadcast(ch->name, c, "*** %s est maintenant modérateur de #%s", next->nick, ch->name);
    }
}

void channels_join(struct client *c, const char *name)
{
    if (strcmp(c->channel, name) == 0) {
        client_send(c, "*** Vous êtes déjà dans #%s", name);
        return;
    }
    int is_default = strcmp(name, DEFAULT_CHANNEL) == 0;
    struct channel *ch = find(name);
    if (!is_default && ch == NULL && nchannels == MAX_CHANNELS) {
        client_send(c, "*** Erreur : trop de salons ouverts");
        return;
    }

    channels_leave(c);                 /* peut supprimer un salon : ch est recalculé */
    broadcast(c->channel, c, "*** %s a quitté #%s pour #%s", c->nick, c->channel, name);
    snprintf(c->channel, sizeof(c->channel), "%s", name);
    ch = find(name);

    if (!is_default && ch == NULL) {
        ch = &channels[nchannels++];
        snprintf(ch->name, sizeof(ch->name), "%s", name);
        ch->moderator_fd = c->fd;
        client_send(c, "*** Salon #%s créé : vous en êtes le modérateur (/kick pseudo)", name);
    } else {
        client_send(c, "*** Vous avez rejoint #%s", name);
    }
    broadcast(name, c, "*** %s a rejoint #%s", c->nick, name);
}

int channels_is_moderator(const char *name, const struct client *c)
{
    struct channel *ch = find(name);
    return ch != NULL && ch->moderator_fd == c->fd;
}
