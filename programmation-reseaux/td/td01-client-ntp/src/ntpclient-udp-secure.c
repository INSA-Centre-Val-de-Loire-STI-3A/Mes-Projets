/* Client NTP durci : même requête que ntpclient-udp-sendto.c, mais la réponse
   n'est acceptée qu'après les vérifications listées dans le README
   (« Points de sécurité »).

   Usage : ./ntpclient-udp-secure [serveur] [port]   (défaut : fr.pool.ntp.org 123) */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/random.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct
{
    uint8_t li_vn_mode;      // 8 bits : LI (2), VN (3), Mode (3)
    uint8_t stratum;         // stratum de l'horloge locale
    uint8_t poll;            // intervalle max entre deux messages
    uint8_t precision;       // précision de l'horloge locale

    uint32_t rootDelay;      // délai aller-retour total
    uint32_t rootDispersion; // erreur max de l'horloge primaire
    uint32_t refId;          // identifiant de l'horloge de référence

    uint32_t refTm_s;        // Reference timestamp : secondes
    uint32_t refTm_f;        //                       fraction de seconde
    uint32_t origTm_s;       // Originate timestamp : secondes
    uint32_t origTm_f;       //                       fraction
    uint32_t rxTm_s;         // Receive timestamp : secondes
    uint32_t rxTm_f;         //                     fraction
    uint32_t txTm_s;         // Transmit timestamp : secondes (le champ utile)
    uint32_t txTm_f;         //                      fraction
} ntp_packet;                // Total : 384 bits = 48 octets

_Static_assert(sizeof(ntp_packet) == 48, "un paquet NTP fait 48 octets");

#define GET_LI(p)    ((p).li_vn_mode >> 6)
#define GET_VN(p)    (((p).li_vn_mode >> 3) & 0x07)
#define GET_MODE(p)  ((p).li_vn_mode & 0x07)
#define NTP_TIMESTAMP_DELTA 2208988800ull  // secondes entre 1900 (NTP) et 1970 (Unix)
#define NTP_MODE_CLIENT 3
#define NTP_MODE_SERVER 4
#define NTP_LI_ALARM    3                  // horloge du serveur non synchronisée
#define TIMEOUT_S       5

static void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

static void reject(const char *why)
{
    fprintf(stderr, "Réponse rejetée : %s\n", why);
    exit(EXIT_FAILURE);
}

/* même adresse IP et même port ? (IPv4 ou IPv6) */
static int same_peer(const struct sockaddr *a, const struct sockaddr *b)
{
    if (a->sa_family != b->sa_family)
        return 0;
    if (a->sa_family == AF_INET) {
        const struct sockaddr_in *x = (const struct sockaddr_in *)a;
        const struct sockaddr_in *y = (const struct sockaddr_in *)b;
        return x->sin_port == y->sin_port
            && x->sin_addr.s_addr == y->sin_addr.s_addr;
    }
    if (a->sa_family == AF_INET6) {
        const struct sockaddr_in6 *x = (const struct sockaddr_in6 *)a;
        const struct sockaddr_in6 *y = (const struct sockaddr_in6 *)b;
        return x->sin6_port == y->sin6_port
            && memcmp(&x->sin6_addr, &y->sin6_addr, sizeof(x->sin6_addr)) == 0;
    }
    return 0;
}

int main(int argc, char **argv)
{
    const char *host_name = (argc > 1) ? argv[1] : "fr.pool.ntp.org";
    const char *port      = (argc > 2) ? argv[2] : "123";

    /* 1. getaddrinfo remplace gethostbyname : IPv4 et IPv6, réentrant */
    struct addrinfo hints = {0}, *res;
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    int rc = getaddrinfo(host_name, port, &hints, &res);
    if (rc != 0) {
        fprintf(stderr, "getaddrinfo(%s) : %s\n", host_name, gai_strerror(rc));
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) error("socket");

    /* 2. délai de réception : on ne reste pas bloqué si personne ne répond */
    struct timeval tv = { .tv_sec = TIMEOUT_S, .tv_usec = 0 };
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        error("setsockopt(SO_RCVTIMEO)");

    /* requête : LI = 0, VN = 4, Mode = 3 (client).
       Transmit timestamp = heure courante + fraction aléatoire : le serveur
       doit la recopier dans l'originate timestamp (RFC 5905, §8). Un attaquant
       qui ne voit pas la requête ne peut pas la deviner. */
    ntp_packet req = {0};
    req.li_vn_mode = (0 << 6) | (4 << 3) | NTP_MODE_CLIENT;
    uint32_t nonce;
    if (getrandom(&nonce, sizeof(nonce), 0) != sizeof(nonce))
        error("getrandom");
    req.txTm_s = htonl((uint32_t)(time(NULL) + NTP_TIMESTAMP_DELTA));
    req.txTm_f = nonce;  // octets aléatoires : l'ordre des octets n'importe pas

    if (sendto(sockfd, &req, sizeof(req), 0, res->ai_addr, res->ai_addrlen) < 0)
        error("sendto");

    /* 3. on attend la réponse ; un datagramme d'une autre source est ignoré */
    ntp_packet resp;
    ssize_t n;
    for (;;) {
        struct sockaddr_storage from;
        socklen_t fromlen = sizeof(from);
        n = recvfrom(sockfd, &resp, sizeof(resp), 0,
                     (struct sockaddr *)&from, &fromlen);
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            fprintf(stderr, "Aucune réponse du serveur en %d s\n", TIMEOUT_S);
            exit(EXIT_FAILURE);
        }
        if (n < 0)
            error("recvfrom");
        if (same_peer((struct sockaddr *)&from, res->ai_addr))
            break;
        fprintf(stderr, "Datagramme ignoré : il ne vient pas du serveur interrogé\n");
    }

    /* 4. taille exacte : sinon les champs lus seraient incomplets */
    if (n != (ssize_t)sizeof(resp))
        reject("taille différente de 48 octets");

    /* 5. originate timestamp = notre transmit timestamp : la réponse
          correspond bien à notre requête, et non à une réponse forgée */
    if (resp.origTm_s != req.txTm_s || resp.origTm_f != req.txTm_f)
        reject("l'originate timestamp ne correspond pas à la requête envoyée");

    /* 6. cohérence du paquet */
    if (GET_MODE(resp) != NTP_MODE_SERVER)
        reject("ce n'est pas une réponse de serveur (mode != 4)");
    if (GET_VN(resp) < 3 || GET_VN(resp) > 4)
        reject("version NTP inattendue");
    if (resp.stratum == 0)
        reject("paquet « Kiss-o'-Death » : le serveur refuse de répondre");
    if (GET_LI(resp) == NTP_LI_ALARM)
        reject("horloge du serveur non synchronisée (LI = 3)");
    if (resp.txTm_s == 0)
        reject("transmit timestamp nul");

    char addr[INET6_ADDRSTRLEN];
    const void *ip = (res->ai_family == AF_INET)
        ? (const void *)&((struct sockaddr_in *)res->ai_addr)->sin_addr
        : (const void *)&((struct sockaddr_in6 *)res->ai_addr)->sin6_addr;
    inet_ntop(res->ai_family, ip, addr, sizeof(addr));

    uint32_t tx_s = ntohl(resp.txTm_s);
    time_t txTm = (time_t)(tx_s - NTP_TIMESTAMP_DELTA);
    struct tm local;
    char date[64];
    strftime(date, sizeof(date), "%a %d %b %Y %H:%M:%S %Z", localtime_r(&txTm, &local));

    printf("Serveur        : %s (%s), stratum %u\n", host_name, addr, resp.stratum);
    printf("Timestamp NTP  : %u\n", tx_s);
    printf("Timestamp Unix : %ld\n", (long)txTm);
    printf("Heure actuelle : %s\n", date);

    freeaddrinfo(res);
    close(sockfd);
    return EXIT_SUCCESS;
}
