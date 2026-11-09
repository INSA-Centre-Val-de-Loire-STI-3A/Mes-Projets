/* Messages et commandes du chat. */

#include "commands.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "accounts.h"
#include "channels.h"

/* préfixe d'un message : « pseudo » dans le salon par défaut, « [#salon] pseudo » ailleurs */
static void prefix(const struct client *c, char *out, size_t size)
{
    if (strcmp(c->channel, DEFAULT_CHANNEL) == 0)
        snprintf(out, size, "%s", c->nick);
    else
        snprintf(out, size, "[#%s] %s", c->channel, c->nick);
}

static void cmd_help(struct client *c)
{
    static const char *lines[] = {
        "*** Commandes :",
        "***   /nickname pseudo [mot-de-passe]   changer de pseudo",
        "***   /register pseudo mot-de-passe     enregistrer un pseudo",
        "***   /unregister pseudo mot-de-passe   supprimer un pseudo enregistré",
        "***   /mp pseudo message                message privé",
        "***   /alerte [pseudo] message          message important (bip, rouge)",
        "***   /send pseudo fichier              envoyer un fichier",
        "***   /red /green /yellow /blue /magenta /cyan message",
        "***   /join salon   /kick pseudo   /who   /date   /exit",
    };
    for (size_t i = 0; i < sizeof(lines) / sizeof(lines[0]); i++)
        client_send(c, "%s", lines[i]);
}

/* /nickname pseudo [mot-de-passe] */
static void cmd_nickname(struct client *c, char *args)
{
    char *nick = next_word(&args), *password = next_word(&args);
    if (nick == NULL || !valid_nick(nick)) {
        client_send(c, "*** Erreur : pseudo invalide (1 à %d caractères parmi A-Z a-z 0-9 _ -)", NICK_MAX);
        return;
    }
    if (strcmp(c->nick, nick) == 0) {
        client_send(c, "*** Vous êtes déjà %s", nick);
        return;
    }
    struct client *other = client_by_nick(nick);

    if (!accounts_exists(nick)) {
        if (other != NULL) {
            client_send(c, "*** Erreur : le pseudo %s est déjà utilisé", nick);
            return;
        }
        client_set_nick(c, nick, 0);
        return;
    }

    /* pseudo enregistré : mot de passe obligatoire */
    if (password == NULL) {
        client_send(c, "*** Erreur : %s est enregistré, utilisez /nickname %s mot-de-passe", nick, nick);
        return;
    }
    if (!accounts_check(nick, password)) {
        client_send(c, "*** Erreur : mot de passe incorrect pour %s", nick);
        return;
    }
    if (other != NULL) {
        if (other->authenticated) {
            client_send(c, "*** Erreur : %s est déjà connecté avec ce pseudo", nick);
            return;
        }
        /* un utilisateur non enregistré occupe le pseudo : il doit en changer */
        char guest[NICK_MAX + 1];
        guest_nick(guest, sizeof(guest));
        client_send(other, "*** Le pseudo %s a été réclamé par son propriétaire", nick);
        client_set_nick(other, guest, 0);
    }
    client_set_nick(c, nick, 1);
}

/* /register pseudo mot-de-passe */
static void cmd_register(struct client *c, char *args)
{
    char *nick = next_word(&args), *password = next_word(&args);
    if (nick == NULL || password == NULL) {
        client_send(c, "*** Usage : /register pseudo mot-de-passe");
        return;
    }
    if (!valid_nick(nick)) {
        client_send(c, "*** Erreur : pseudo invalide");
        return;
    }
    if (strlen(password) < 4 || strlen(password) > PASSWORD_MAX) {
        client_send(c, "*** Erreur : le mot de passe doit faire 4 à %d caractères", PASSWORD_MAX);
        return;
    }
    if (accounts_exists(nick)) {
        client_send(c, "*** Erreur : %s est déjà enregistré", nick);
        return;
    }
    if (accounts_register(nick, password) == -1) {
        client_send(c, "*** Erreur : enregistrement impossible");
        return;
    }
    printf("Pseudo %s enregistré\n", nick);
    if (strcmp(c->nick, nick) == 0)
        c->authenticated = 1;
    client_send(c, "*** Pseudo %s enregistré%s", nick,
                strcmp(c->nick, nick) == 0 ? "" : " (utilisez /nickname pseudo mot-de-passe)");
}

/* /unregister pseudo mot-de-passe */
static void cmd_unregister(struct client *c, char *args)
{
    char *nick = next_word(&args), *password = next_word(&args);
    if (nick == NULL || password == NULL) {
        client_send(c, "*** Usage : /unregister pseudo mot-de-passe");
        return;
    }
    if (!accounts_check(nick, password)) {
        client_send(c, "*** Erreur : pseudo non enregistré ou mot de passe incorrect");
        return;
    }
    if (accounts_unregister(nick) == -1) {
        client_send(c, "*** Erreur : suppression impossible");
        return;
    }
    printf("Pseudo %s supprimé\n", nick);
    struct client *owner = client_by_nick(nick);
    if (owner != NULL)
        owner->authenticated = 0;
    client_send(c, "*** Pseudo %s supprimé", nick);
}

static void cmd_date(struct client *c)
{
    time_t now = time(NULL);
    struct tm tm;
    char buf[64];
    strftime(buf, sizeof(buf), "%A %d %B %Y %H:%M:%S %Z", localtime_r(&now, &tm));
    client_send(c, "*** Heure du serveur : %s", buf);
}

/* /mp pseudo message */
static void cmd_mp(struct client *c, char *args)
{
    char *nick = next_word(&args);
    char *msg = rest_of_line(&args);
    if (nick == NULL || *msg == '\0') {
        client_send(c, "*** Usage : /mp pseudo message");
        return;
    }
    struct client *dest = client_by_nick(nick);
    if (dest == NULL) {
        client_send(c, "*** Erreur : %s n'est pas connecté", nick);
        return;
    }
    client_send(dest, "private message from : %s : %s", c->nick, msg);
    client_send(c, "private message to : %s : %s", nick, msg);
}

/* /alerte message  ou  /alerte pseudo message.
   \a fait biper le terminal, ANSI_ALERT affiche en gras rouge. */
static void cmd_alerte(struct client *c, char *args)
{
    char all[MSG_MAX];
    snprintf(all, sizeof(all), "%s", rest_of_line(&args));   /* avant découpage */
    char *first = next_word(&args);
    char *msg = rest_of_line(&args);
    if (first == NULL) {
        client_send(c, "*** Usage : /alerte [pseudo] message");
        return;
    }
    struct client *dest = client_by_nick(first);
    if (dest != NULL && *msg != '\0') {
        client_send(dest, "\a" ANSI_ALERT "[ALERTE de %s] %s" ANSI_RESET, c->nick, msg);
        client_send(c, "*** Alerte envoyée à %s", dest->nick);
        return;
    }
    /* le premier mot n'est pas un pseudo connecté : alerte pour tout le salon */
    broadcast(c->channel, NULL, "\a" ANSI_ALERT "[ALERTE de %s] %s" ANSI_RESET, c->nick, all);
}

/* /send pseudo nom taille, suivi de taille octets (le client lit le fichier) */
static void cmd_send(struct client *c, char *args)
{
    char *nick = next_word(&args), *name = next_word(&args), *size_s = next_word(&args);
    size_t size;
    if (nick == NULL || name == NULL || !parse_size(size_s, &size)) {
        /* on ne sait pas combien d'octets suivent : impossible de se resynchroniser */
        client_send(c, "*** Erreur : en-tête /send invalide (taille max %d octets), déconnexion", FILE_MAX);
        c->closing = 1;
        return;
    }
    c->file_left = size;
    c->file_dest = -1;               /* par défaut, les octets sont lus puis jetés */

    struct client *dest = client_by_nick(nick);
    if (c->nick[0] == '\0') {
        client_send(c, "*** Choisissez d'abord un pseudo : /nickname pseudo (fichier ignoré)");
    } else if (dest == NULL || dest == c) {
        client_send(c, "*** Erreur : destinataire %s introuvable, fichier ignoré", nick);
    } else if (!valid_filename(name)) {
        client_send(c, "*** Erreur : nom de fichier refusé, fichier ignoré");
    } else {
        client_send(dest, "/file %s %s %zu", c->nick, name, size);
        if (!dest->closing) c->file_dest = dest->fd;
        client_send(c, "*** Envoi de %s (%zu octets) à %s", name, size, nick);
        printf("%s envoie %s (%zu octets) à %s\n", c->nick, name, size, nick);
    }
}

/* /red message, /blue message... */
static void cmd_color(struct client *c, const char *color, char *args)
{
    char *msg = rest_of_line(&args);
    if (*msg == '\0') {
        client_send(c, "*** Usage : /couleur message");
        return;
    }
    char who[NICK_MAX + CHANNEL_MAX + 8];
    prefix(c, who, sizeof(who));
    broadcast(c->channel, c, "%s : %s%s" ANSI_RESET, who, color, msg);
}

/* /kick pseudo : réservé au modérateur du salon */
static void cmd_kick(struct client *c, char *args)
{
    char *nick = next_word(&args);
    if (nick == NULL) {
        client_send(c, "*** Usage : /kick pseudo");
        return;
    }
    if (!channels_is_moderator(c->channel, c)) {
        client_send(c, "*** Erreur : vous n'êtes pas modérateur de #%s", c->channel);
        return;
    }
    struct client *target = client_by_nick(nick);
    if (target == NULL || target == c || strcmp(target->channel, c->channel) != 0) {
        client_send(c, "*** Erreur : %s n'est pas dans #%s", nick, c->channel);
        return;
    }
    char from[CHANNEL_MAX + 1];
    snprintf(from, sizeof(from), "%s", c->channel);
    client_send(target, "*** Vous avez été expulsé de #%s par %s", from, c->nick);
    broadcast(from, target, "*** %s a été expulsé par %s", target->nick, c->nick);
    channels_join(target, DEFAULT_CHANNEL);
}

static void cmd_who(struct client *c)
{
    client_send(c, "*** Connectés dans #%s :", c->channel);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        struct client *m = &clients[i];
        if (m->fd != -1 && m->nick[0] != '\0' && strcmp(m->channel, c->channel) == 0)
            client_send(c, "***   %s%s%s", m->nick, m->authenticated ? " (enregistré)" : "",
                        channels_is_moderator(c->channel, m) ? " (modérateur)" : "");
    }
}

void handle_line(struct client *c, char *line)
{
    if (*line == '\0') return;

    sanitize(line);                  /* pas de séquences ANSI venant des utilisateurs,
                                        ni dans le chat ni dans le journal du serveur */

    /* journal : sans les mots de passe */
    if (strncmp(line, "/register", 9) == 0 || strncmp(line, "/unregister", 11) == 0
        || strncmp(line, "/nickname", 9) == 0)
        printf("%s : %.*s [arguments masqués]\n", c->nick[0] ? c->nick : c->addr,
               (int)strcspn(line, " "), line);
    else
        printf("%s : %s\n", c->nick[0] ? c->nick : c->addr, line);

    if (line[0] != '/') {
        if (c->nick[0] == '\0') {
            client_send(c, "*** Choisissez d'abord un pseudo : /nickname pseudo");
            return;
        }
        char who[NICK_MAX + CHANNEL_MAX + 8];
        prefix(c, who, sizeof(who));
        broadcast(c->channel, c, "%s : %s", who, line);
        return;
    }

    char *args = line + 1;
    char *cmd = next_word(&args);
    if (cmd == NULL) return;

    /* commandes possibles sans pseudo */
    if (strcmp(cmd, "nickname") == 0)   { cmd_nickname(c, args);   return; }
    if (strcmp(cmd, "register") == 0)   { cmd_register(c, args);   return; }
    if (strcmp(cmd, "unregister") == 0) { cmd_unregister(c, args); return; }
    if (strcmp(cmd, "date") == 0)       { cmd_date(c);             return; }
    if (strcmp(cmd, "help") == 0)       { cmd_help(c);             return; }
    if (strcmp(cmd, "exit") == 0) {
        client_send(c, "*** Au revoir");
        c->closing = 1;
        return;
    }

    if (c->nick[0] == '\0' && strcmp(cmd, "send") != 0) {
        client_send(c, "*** Choisissez d'abord un pseudo : /nickname pseudo");
        return;
    }

    const char *color = ansi_color(cmd);
    if (color != NULL)                      cmd_color(c, color, args);
    else if (strcmp(cmd, "mp") == 0)        cmd_mp(c, args);
    else if (strcmp(cmd, "alerte") == 0)    cmd_alerte(c, args);
    else if (strcmp(cmd, "send") == 0)      cmd_send(c, args);
    else if (strcmp(cmd, "kick") == 0)      cmd_kick(c, args);
    else if (strcmp(cmd, "who") == 0)       cmd_who(c);
    else if (strcmp(cmd, "join") == 0) {
        char *name = next_word(&args);
        if (name != NULL && name[0] == '#') name++;
        if (name == NULL || !valid_channel(name))
            client_send(c, "*** Usage : /join salon (1 à %d caractères parmi A-Z a-z 0-9 _ -)", CHANNEL_MAX);
        else
            channels_join(c, name);
    }
    else client_send(c, "*** Erreur : commande inconnue /%s (/help pour la liste)", cmd);
}
