/* TD3, ex. 2 : télécharger la totalité d'un fichier servi par
       cat test.txt | nc -l 8000 -N

   Un seul recv() ne suffit pas : TCP est un flux, les 700 Ko arrivent en
   plusieurs morceaux. On lit en boucle jusqu'à ce que recv() renvoie 0
   (le serveur a fermé la connexion = fin du fichier).

   Usage : ./download_client [ip] [port] [fichier]   (défaut : 127.0.0.1 8000 download.txt) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFLEN 4096

static void stop(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

/* écrit tout le tampon : write() peut écrire moins que demandé */
static void write_all(int fd, const char *buf, size_t len)
{
    while (len > 0) {
        ssize_t n = write(fd, buf, len);
        if (n == -1) {
            if (errno == EINTR) continue;
            stop("write");
        }
        buf += n;
        len -= (size_t)n;
    }
}

int main(int argc, char **argv)
{
    const char *ip   = (argc > 1) ? argv[1] : "127.0.0.1";
    int         port = (argc > 2) ? atoi(argv[2]) : 8000;
    const char *path = (argc > 3) ? argv[3] : "download.txt";
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "port invalide\n");
        return EXIT_FAILURE;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) stop("socket");

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port   = htons((uint16_t)port);
    if (inet_pton(AF_INET, ip, &serv.sin_addr) != 1) {
        fprintf(stderr, "adresse IPv4 invalide : %s\n", ip);
        return EXIT_FAILURE;
    }
    if (connect(sockfd, (struct sockaddr *)&serv, sizeof(serv)) == -1) stop("connect");

    int out = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out == -1) stop(path);

    char buf[BUFLEN];
    size_t total = 0, chunks = 0;
    for (;;) {
        ssize_t n = recv(sockfd, buf, sizeof(buf), 0);
        if (n == -1) {
            if (errno == EINTR) continue;
            stop("recv");
        }
        if (n == 0) break;                 /* fin du fichier */
        write_all(out, buf, (size_t)n);
        total += (size_t)n;
        chunks++;
    }

    if (close(out) == -1) stop("close");
    close(sockfd);
    printf("%zu octets reçus en %zu recv(), enregistrés dans %s\n", total, chunks, path);
    return 0;
}
