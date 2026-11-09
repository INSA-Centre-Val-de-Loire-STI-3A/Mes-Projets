/* Outils réseau communs : création des sockets, envoi complet, lecture par ligne. */

#include "net.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "protocol.h"

void stop(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

uint16_t parse_port(const char *s)
{
    char *end;
    errno = 0;
    long p = strtol(s, &end, 10);
    if (errno != 0 || *s == '\0' || *end != '\0' || p < 1 || p > 65535) {
        fprintf(stderr, "Port invalide : %s\n", s);
        exit(EXIT_FAILURE);
    }
    return (uint16_t)p;
}

int tcp_listen(uint16_t port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) stop("socket");

    int on = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
        stop("setsockopt");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) stop("bind");
    if (listen(fd, SOMAXCONN) == -1) stop("listen");
    return fd;
}

int tcp_connect(const char *host, uint16_t port)
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    char service[8];
    snprintf(service, sizeof(service), "%u", port);

    int rc = getaddrinfo(host, service, &hints, &res);
    if (rc != 0) {
        fprintf(stderr, "Adresse %s : %s\n", host, gai_strerror(rc));
        exit(EXIT_FAILURE);
    }
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == -1) stop("socket");
    if (connect(fd, res->ai_addr, res->ai_addrlen) == -1) stop("connect");
    freeaddrinfo(res);
    return fd;
}

int send_all(int fd, const void *buf, size_t len)
{
    const char *p = buf;
    while (len > 0) {
        /* MSG_NOSIGNAL : un pair déconnecté donne EPIPE au lieu de tuer le processus */
        ssize_t n = send(fd, p, len, MSG_NOSIGNAL);
        if (n == -1) {
            if (errno == EINTR) continue;
            return -1;
        }
        p   += n;
        len -= (size_t)n;
    }
    return 0;
}

int sendf(int fd, const char *fmt, ...)
{
    char buf[MSG_MAX + 256];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
    va_end(ap);
    if (n < 0) return -1;
    size_t len = (size_t)n < sizeof(buf) - 1 ? (size_t)n : sizeof(buf) - 2;
    buf[len++] = '\n';
    return send_all(fd, buf, len);
}

ssize_t linebuf_fill(int fd, struct linebuf *lb)
{
    size_t space = sizeof(lb->data) - lb->len;
    if (space == 0) return -2;
    ssize_t n;
    do {
        n = recv(fd, lb->data + lb->len, space, 0);
    } while (n == -1 && errno == EINTR);
    if (n > 0) lb->len += (size_t)n;
    return n;
}

int linebuf_getline(struct linebuf *lb, char *line, size_t size)
{
    char *nl = memchr(lb->data, '\n', lb->len);
    if (nl == NULL) return 0;

    size_t l = (size_t)(nl - lb->data);
    size_t copy = l < size - 1 ? l : size - 1;
    memcpy(line, lb->data, copy);
    line[copy] = '\0';
    if (copy > 0 && line[copy - 1] == '\r')   /* clients qui envoient \r\n */
        line[copy - 1] = '\0';

    lb->len -= l + 1;
    memmove(lb->data, nl + 1, lb->len);
    return 1;
}

size_t linebuf_take(struct linebuf *lb, void *out, size_t max)
{
    size_t n = lb->len < max ? lb->len : max;
    memcpy(out, lb->data, n);
    lb->len -= n;
    memmove(lb->data, lb->data + n, lb->len);
    return n;
}
