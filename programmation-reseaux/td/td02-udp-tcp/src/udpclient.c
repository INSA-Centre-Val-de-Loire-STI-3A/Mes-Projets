/* TD2, ex. 8-13 : client UDP « PING-PONG ».
   Envoie PING au serveur 127.0.0.1:1234, affiche la réponse, recommence
   (arrêt par Ctrl+C). */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER  "127.0.0.1"
#define PORT    1234
#define BUFLEN  512
#define MSG     "PING"

/* ex. 9 */
static void stop(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

int main(void)
{
    /* ex. 10 : socket UDP IPv4 */
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) stop("socket");

    /* ex. 11 : adresse du serveur */
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port   = htons(PORT);
    if (inet_pton(AF_INET, SERVER, &serv.sin_addr) != 1) stop("inet_pton");

    /* ex. 12 : boucle infinie */
    char buf[BUFLEN + 1];
    for (;;) {
        if (sendto(sockfd, MSG, strlen(MSG), 0, (struct sockaddr *)&serv, sizeof(serv)) == -1)
            stop("sendto");
        printf("Envoyé %s\n", MSG);

        /* pas de connect() en UDP : n'importe qui pourrait répondre.
           On n'accepte que la réponse qui vient du serveur interrogé. */
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(from);
        ssize_t n = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *)&from, &fromlen);
        if (n == -1) stop("recvfrom");
        if (from.sin_addr.s_addr != serv.sin_addr.s_addr || from.sin_port != serv.sin_port) {
            fprintf(stderr, "Datagramme ignoré : il ne vient pas du serveur\n");
            continue;
        }
        buf[n] = '\0';
        printf("Reçu %s\n", buf);
    }
}
