/* Serveur IRC (ex. 17 et suivants).
   Un seul processus : poll() surveille la socket d'écoute et tous les clients.

   Usage : ./irc_server [port] [fichier-comptes]   (défaut : 8000 data/accounts.txt) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "net.h"
#include "protocol.h"
#include "clients.h"
#include "accounts.h"
#include "commands.h"

static void accept_client(int listenfd)
{
    struct sockaddr_in cli;
    socklen_t len = sizeof(cli);
    int fd = accept(listenfd, (struct sockaddr *)&cli, &len);
    if (fd == -1) {
        perror("accept");
        return;
    }
    char ip[INET_ADDRSTRLEN], addr[32];
    inet_ntop(AF_INET, &cli.sin_addr, ip, sizeof(ip));
    snprintf(addr, sizeof(addr), "%s:%u", ip, ntohs(cli.sin_port));

    struct client *c = client_add(fd, addr);
    if (c == NULL) {
        sendf(fd, "*** Serveur plein, réessayez plus tard");
        close(fd);
        return;
    }
    printf("Nouveau client %s\n", addr);
    client_send(c, "*** Bienvenue sur le chat. Choisissez un pseudo : /nickname pseudo (/help pour l'aide)");
}

/* relaie les octets d'un /send en cours ; 1 si le fichier est terminé */
static void relay_file(struct client *c)
{
    char buf[LINEBUF_SIZE];
    size_t n = linebuf_take(&c->in, buf, c->file_left < sizeof(buf) ? c->file_left : sizeof(buf));
    if (n == 0) return;
    if (c->file_dest != -1 && send_all(c->file_dest, buf, n) == -1) {
        struct client *dest = client_by_fd(c->file_dest);
        if (dest != NULL) dest->closing = 1;
        c->file_dest = -1;
    }
    c->file_left -= n;
    if (c->file_left == 0) {
        client_send(c, c->file_dest != -1 ? "*** Fichier transmis" : "*** Fichier ignoré");
        c->file_dest = -1;
    }
}

/* lit ce qui est arrivé et traite chaque ligne complète */
static void read_client(struct client *c)
{
    ssize_t n = linebuf_fill(c->fd, &c->in);
    if (n == 0 || n == -1) {          /* déconnexion ou erreur */
        c->closing = 1;
        return;
    }

    char line[MSG_MAX + 1];
    while (!c->closing && c->fd != -1) {
        if (c->file_left > 0) {        /* octets bruts d'un fichier */
            if (c->in.len == 0) break;
            relay_file(c);
        } else if (linebuf_getline(&c->in, line, sizeof(line))) {
            handle_line(c, line);
        } else {
            break;
        }
    }

    /* tampon plein sans fin de ligne : ligne beaucoup trop longue */
    if (c->file_left == 0 && c->in.len == sizeof(c->in.data)) {
        client_send(c, "*** Erreur : ligne trop longue (%d caractères max), ignorée", MSG_MAX);
        c->in.len = 0;
    }
}

int main(int argc, char **argv)
{
    uint16_t port = (argc > 1) ? parse_port(argv[1]) : 8000;
    const char *accounts_path = (argc > 2) ? argv[2] : "data/accounts.txt";

    signal(SIGPIPE, SIG_IGN);
    setvbuf(stdout, NULL, _IOLBF, 0);  /* journal lisible même redirigé */

    if (argc <= 2 && mkdir("data", 0700) == -1 && errno != EEXIST)
        stop("mkdir data");
    accounts_load(accounts_path);

    clients_init();
    int listenfd = tcp_listen(port);
    printf("Serveur IRC en écoute sur le port %u\n", port);

    struct pollfd fds[MAX_CLIENTS + 1];
    struct client *owner[MAX_CLIENTS + 1];
    for (;;) {
        /* fds[0] = écoute, puis un pollfd par client connecté */
        nfds_t nfds = 0;
        fds[nfds].fd = listenfd;
        fds[nfds].events = POLLIN;
        owner[nfds++] = NULL;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd == -1) continue;
            fds[nfds].fd = clients[i].fd;
            fds[nfds].events = POLLIN;
            owner[nfds++] = &clients[i];
        }

        if (poll(fds, nfds, -1) == -1) {
            if (errno == EINTR) continue;
            stop("poll");
        }

        if (fds[0].revents & POLLIN)
            accept_client(listenfd);
        for (nfds_t i = 1; i < nfds; i++)
            if (fds[i].revents & (POLLIN | POLLHUP | POLLERR) && owner[i]->fd != -1 && !owner[i]->closing)
                read_client(owner[i]);

        /* fermeture en fin de tour : aucun pointeur vers un client supprimé
           n'est utilisé pendant le traitement */
        for (int i = 0; i < MAX_CLIENTS; i++)
            if (clients[i].fd != -1 && clients[i].closing)
                client_remove(&clients[i]);
    }
}
