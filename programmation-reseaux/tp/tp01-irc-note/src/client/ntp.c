/* Requête NTP du TD1, version durcie (nonce, expéditeur, délai, cohérence). */

#include "ntp.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/random.h>
#include <arpa/inet.h>

typedef struct {
    uint8_t  li_vn_mode, stratum, poll, precision;
    uint32_t rootDelay, rootDispersion, refId;
    uint32_t refTm_s, refTm_f, origTm_s, origTm_f, rxTm_s, rxTm_f, txTm_s, txTm_f;
} ntp_packet;

_Static_assert(sizeof(ntp_packet) == 48, "un paquet NTP fait 48 octets");

#define NTP_TIMESTAMP_DELTA 2208988800ull
#define TIMEOUT_S 2

static time_t offset = 0;              /* heure NTP - heure locale, en secondes */

static int query(const char *server, time_t *ntp_time)
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    int rc = getaddrinfo(server, "123", &hints, &res);
    if (rc != 0) {
        fprintf(stderr, "NTP : %s : %s\n", server, gai_strerror(rc));
        return -1;
    }

    int ret = -1;
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == -1) { perror("NTP socket"); goto out; }
    struct timeval tv = { .tv_sec = TIMEOUT_S, .tv_usec = 0 };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    /* connect() en UDP : le noyau ne nous remonte que les datagrammes du serveur */
    if (connect(fd, res->ai_addr, res->ai_addrlen) == -1) { perror("NTP connect"); goto out; }

    ntp_packet req, resp;
    memset(&req, 0, sizeof(req));
    req.li_vn_mode = (4 << 3) | 3;                 /* VN = 4, mode client */
    uint32_t nonce;
    if (getrandom(&nonce, sizeof(nonce), 0) != sizeof(nonce)) goto out;
    req.txTm_s = htonl((uint32_t)(time(NULL) + NTP_TIMESTAMP_DELTA));
    req.txTm_f = nonce;

    if (send(fd, &req, sizeof(req), 0) != sizeof(req)) { perror("NTP send"); goto out; }
    ssize_t n = recv(fd, &resp, sizeof(resp), 0);
    if (n == -1) {
        fprintf(stderr, "NTP : %s\n", errno == EAGAIN ? "pas de réponse" : strerror(errno));
        goto out;
    }
    if (n != sizeof(resp)
        || resp.origTm_s != req.txTm_s || resp.origTm_f != req.txTm_f   /* réponse à notre requête */
        || (resp.li_vn_mode & 7) != 4 || resp.stratum == 0 || (resp.li_vn_mode >> 6) == 3) {
        fprintf(stderr, "NTP : réponse rejetée\n");
        goto out;
    }
    *ntp_time = (time_t)(ntohl(resp.txTm_s) - NTP_TIMESTAMP_DELTA);
    ret = 0;
out:
    if (fd != -1) close(fd);
    freeaddrinfo(res);
    return ret;
}

int ntp_init(const char *server)
{
    time_t t;
    if (query(server, &t) == -1)
        return -1;
    offset = t - time(NULL);
    return 0;
}

time_t ntp_now(void)
{
    return time(NULL) + offset;
}
