#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/udp.h>

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

void error(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc , char **argv)
{
    ntp_packet packet = {0} ;
    int sockfd;
    int portno = 123;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    char *host_name = "fr.pool.ntp.org";
    struct hostent *server;
    struct sockaddr_in serv_addr;

    bzero(&serv_addr, sizeof serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    server = gethostbyname(host_name);
    if(server == NULL) error("ERROR no such host");
    bcopy(server->h_addr_list[0], &serv_addr.sin_addr.s_addr, server->h_length);

    if(sockfd < 0)
        error("ERROR opening socket");

    SET_LI(packet, 0);
    SET_VN(packet, 3);
    SET_MODE(packet, 3);
    return EXIT_SUCCESS;
}

