/* TD3, ex. 3 : serveur TCP multi-client sur le port 8000.
   Chaque message reçu d'un client est renvoyé à tous les autres.

   Un seul processus, sans thread : poll() surveille la socket d'écoute
   (nouveau client) et toutes les sockets clientes (données ou déconnexion).
   C'est la base du serveur IRC du TP.

   Test : ./broadcast_server & puis « nc localhost 8000 » dans plusieurs terminaux. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT        8000
#define MAX_CLIENTS 64
#define BUFLEN      1024

static void stop(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

/* envoie tout le tampon ; -1 si le client est injoignable */
static int send_all(int fd, const char *buf, size_t len)
{
    while (len > 0) {
        ssize_t n = send(fd, buf, len, MSG_NOSIGNAL);
        if (n == -1) {
            if (errno == EINTR) continue;
            return -1;
        }
        buf += n;
        len -= (size_t)n;
    }
    return 0;
}

int main(void)
{
    signal(SIGPIPE, SIG_IGN);

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) stop("socket");
    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
        stop("setsockopt");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) stop("bind");
    if (listen(listenfd, SOMAXCONN) == -1) stop("listen");
    printf("Serveur de diffusion en écoute sur le port %d\n", PORT);

    /* fds[0] = socket d'écoute, fds[1..nfds-1] = clients */
    struct pollfd fds[MAX_CLIENTS + 1];
    int nfds = 1;
    fds[0].fd = listenfd;
    fds[0].events = POLLIN;

    char buf[BUFLEN];
    for (;;) {
        if (poll(fds, (nfds_t)nfds, -1) == -1) {
            if (errno == EINTR) continue;
            stop("poll");
        }

        /* nouveau client */
        if (fds[0].revents & POLLIN) {
            struct sockaddr_in cli;
            socklen_t len = sizeof(cli);
            int fd = accept(listenfd, (struct sockaddr *)&cli, &len);
            if (fd == -1) {
                perror("accept");
            } else if (nfds > MAX_CLIENTS) {
                send_all(fd, "Serveur plein\n", 14);
                close(fd);
            } else {
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &cli.sin_addr, ip, sizeof(ip));
                printf("Client %d connecté depuis %s:%d\n", fd, ip, ntohs(cli.sin_port));
                fds[nfds].fd = fd;
                fds[nfds].events = POLLIN;
                fds[nfds].revents = 0;
                nfds++;
            }
        }

        /* données ou déconnexion des clients */
        for (int i = 1; i < nfds; i++) {
            if (!(fds[i].revents & (POLLIN | POLLHUP | POLLERR)))
                continue;
            ssize_t n = recv(fds[i].fd, buf, sizeof(buf), 0);
            if (n <= 0) {
                if (n == -1) perror("recv");
                printf("Client %d déconnecté\n", fds[i].fd);
                close(fds[i].fd);
                fds[i] = fds[--nfds];   /* on bouche le trou avec le dernier */
                i--;                    /* et on traite la case déplacée */
                continue;
            }
            printf("Client %d : %.*s", fds[i].fd, (int)n, buf);
            if (buf[n - 1] != '\n') putchar('\n');
            for (int j = 1; j < nfds; j++)
                if (j != i && send_all(fds[j].fd, buf, (size_t)n) == -1)
                    perror("send");     /* le client sera retiré à son prochain poll */
        }
    }
}
