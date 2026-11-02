#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>

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

#define SET_LI(packet, li)   (uint8_t)(packet.li_vn_mode |= (li << 6))
#define SET_VN(packet, vn)   (uint8_t)(packet.li_vn_mode |= (vn << 3))
#define SET_MODE(packet, mode) (uint8_t)(packet.li_vn_mode |= (mode << 0))
#define NTP_TIMESTAMP_DELTA 2208988800ull  // secondes entre 1900 (NTP) et 1970 (Unix)
#define NTP_PORT 123

void error(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(void)
{
    ntp_packet packet = {0};
    SET_LI(packet, 0);  SET_VN(packet, 3);  SET_MODE(packet, 3);  // client NTPv3

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    char *host_name = "fr.pool.ntp.org";
    struct hostent *server = gethostbyname(host_name);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host: %s\n", host_name);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(NTP_PORT);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

    /* en UDP, connect n'établit pas de connexion : il fixe le destinataire par défaut,
       ce qui permet d'utiliser write/read au lieu de sendto/recvfrom */
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    ssize_t n = write(sockfd, &packet, sizeof(ntp_packet));
    if (n < 0) error("ERROR writing to socket");

    n = read(sockfd, &packet, sizeof(ntp_packet));
    if (n < 0) error("ERROR reading from socket");

    packet.txTm_s = ntohl(packet.txTm_s);  // ordre réseau (big endian) -> ordre de la machine
    time_t txTm = (time_t)(packet.txTm_s - NTP_TIMESTAMP_DELTA);
    printf("Timestamp NTP  : %u\n", packet.txTm_s);
    printf("Timestamp Unix : %ld\n", (long)txTm);
    printf("Heure actuelle : %s", ctime(&txTm));

    close(sockfd);
    return EXIT_SUCCESS;
}
