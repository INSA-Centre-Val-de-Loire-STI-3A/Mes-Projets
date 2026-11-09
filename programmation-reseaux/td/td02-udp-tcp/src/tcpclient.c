/* TD2, ex. 24-29 : client TCP echo.
   Se connecte à 127.0.0.1:1234, envoie ECHO, affiche la réponse, 1000 fois. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER  "127.0.0.1"
#define PORT    1234
#define BUFLEN  512
#define MSG     "ECHO"
#define ITER    1000

/* ex. 25 */
static void stop(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

int main(void)
{
    /* ex. 26 : socket TCP IPv4 */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) stop("socket");

    /* ex. 27 : adresse du serveur */
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(PORT);
    if (inet_pton(AF_INET, SERVER, &serv_addr.sin_addr) != 1) stop("inet_pton");

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        stop("connect");
    printf("Connecté à %s:%d\n", SERVER, PORT);

    /* ex. 28 : 1000 échanges */
    char message[BUFLEN + 1];
    for (int i = 0; i < ITER; i++) {
        printf("Envoi %s\n", MSG);
        if (send(sockfd, MSG, strlen(MSG), MSG_NOSIGNAL) == -1) stop("send");

        ssize_t n = recv(sockfd, message, BUFLEN, 0);
        if (n == -1) stop("recv");
        if (n == 0) {
            printf("Le serveur a fermé la connexion\n");
            break;
        }
        message[n] = '\0';
        printf("Recv : %s\n", message);
    }

    close(sockfd);
    return 0;
}
