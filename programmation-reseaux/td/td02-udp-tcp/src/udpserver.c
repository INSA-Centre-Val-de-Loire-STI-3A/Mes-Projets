/* TD2, ex. 1-7 : serveur UDP « PING-PONG ».
   Écoute 127.0.0.1:1234, affiche chaque message reçu et son émetteur,
   attend 1 s puis répond PONG.

   Test : echo -n coucou | nc -4u -w0 localhost 1234 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER  "127.0.0.1"
#define PORT    1234
#define BUFLEN  512
#define REPLY   "PONG"

/* ex. 2 : arrêt sur erreur d'un appel système */
static void stop(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

int main(void)
{
    /* ex. 3 : socket UDP IPv4 */
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) stop("socket");

    /* ex. 4 : adresse 127.0.0.1:1234 */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(PORT);
    if (inet_pton(AF_INET, SERVER, &addr.sin_addr) != 1) stop("inet_pton");

    /* ex. 5 : association de la socket à l'adresse */
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) stop("bind");
    printf("Serveur UDP en écoute sur %s:%d\n", SERVER, PORT);

    /* ex. 6 : boucle infinie (arrêt par Ctrl+C) */
    char buf[BUFLEN + 1];
    for (;;) {
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(from);
        ssize_t n = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *)&from, &fromlen);
        if (n == -1) stop("recvfrom");
        buf[n] = '\0';   // un datagramme n'est pas une chaîne : on la termine

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &from.sin_addr, ip, sizeof(ip));
        printf("Reçu « %s » de %s:%d\n", buf, ip, ntohs(from.sin_port));

        sleep(1);
        if (sendto(sockfd, REPLY, strlen(REPLY), 0, (struct sockaddr *)&from, fromlen) == -1)
            stop("sendto");
    }
}
