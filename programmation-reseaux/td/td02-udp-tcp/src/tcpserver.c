/* TD2, ex. 14-23 : serveur TCP echo mono-client.
   Écoute 127.0.0.1:1234, accepte un client, renvoie chaque message reçu
   (1000 fois au plus).

   Ex. 23 : avec « echo -n coucou | nc -4 -w0 localhost 1234 », nc envoie
   « coucou » puis ferme la connexion. Le serveur d'origine boucle alors
   sans fin : recv() renvoie 0 (fin de flux) à chaque tour, le serveur
   affiche un message vide et tente un send() vers une connexion fermée
   (EPIPE / SIGPIPE). Correction : tester le retour de recv(), 0 = le client
   s'est déconnecté, on ferme la socket du client et on en attend un autre. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER  "127.0.0.1"
#define PORT    1234
#define BUFLEN  512
#define BACKLOG 5
#define ITER    1000

/* ex. 15 */
static void stop(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

int main(void)
{
    /* ex. 16 : socket TCP IPv4 */
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) stop("socket");

    /* relancer le serveur tout de suite sans attendre la fin de TIME_WAIT */
    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
        stop("setsockopt");

    /* ex. 17 : adresse 127.0.0.1:1234 */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(PORT);
    if (inet_pton(AF_INET, SERVER, &addr.sin_addr) != 1) stop("inet_pton");

    /* ex. 18 et 19 */
    if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) stop("bind");
    if (listen(listenfd, BACKLOG) == -1) stop("listen");
    printf("Serveur TCP echo en écoute sur %s:%d\n", SERVER, PORT);

    char buf[BUFLEN + 1];
    int done = 0;
    while (done < ITER) {
        /* ex. 20 : un client à la fois */
        struct sockaddr_in cli;
        socklen_t clilen = sizeof(cli);
        int connfd = accept(listenfd, (struct sockaddr *)&cli, &clilen);
        if (connfd == -1) stop("accept");
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli.sin_addr, ip, sizeof(ip));
        printf("Client connecté : %s:%d\n", ip, ntohs(cli.sin_port));

        /* ex. 21 : 1000 itérations recv / affichage / send */
        while (done < ITER) {
            ssize_t n = recv(connfd, buf, BUFLEN, 0);
            if (n == -1) stop("recv");
            if (n == 0) {                 /* ex. 23 : fin de flux */
                printf("Client déconnecté\n");
                break;
            }
            buf[n] = '\0';
            printf("Reçu : %s\n", buf);
            /* MSG_NOSIGNAL : pas de SIGPIPE si le client vient de partir */
            if (send(connfd, buf, (size_t)n, MSG_NOSIGNAL) == -1) {
                perror("send");
                break;
            }
            done++;
        }
        close(connfd);
    }

    close(listenfd);
    return 0;
}
