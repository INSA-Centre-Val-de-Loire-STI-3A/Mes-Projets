/* Client IRC (ex. 18 et suivants).

   Usage : ./irc_client ip port [serveur-ntp]   (défaut NTP : fr.pool.ntp.org,
           « none » pour utiliser l'horloge locale)

   Un seul processus : poll() surveille le clavier et la socket. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <libgen.h>
#include <sys/stat.h>

#include "net.h"
#include "protocol.h"
#include "ntp.h"
#include "ui.h"

static int sockfd;
static struct linebuf sock_in, kbd_in;

/* fichier en cours de réception */
static int    file_fd = -1;
static size_t file_left = 0;
static char   file_path[FILENAME_MAX_LEN + 8];
static char   file_from[NICK_MAX + 1];

/* /send pseudo chemin : en-tête puis contenu du fichier */
static void send_file(char *args)
{
    char *nick = next_word(&args), *path = next_word(&args);
    if (nick == NULL || path == NULL) {
        ui_info("Usage : /send pseudo fichier");
        return;
    }
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        ui_info("Impossible d'ouvrir %s : %s", path, strerror(errno));
        return;
    }
    struct stat st;
    if (fstat(fd, &st) == -1 || !S_ISREG(st.st_mode) || st.st_size > FILE_MAX) {
        ui_info("%s : fichier ordinaire de %d octets maximum attendu", path, FILE_MAX);
        close(fd);
        return;
    }
    const char *name = basename(path);        /* on n'envoie pas le chemin local */
    if (!valid_filename(name)) {
        ui_info("Nom de fichier refusé (caractères autorisés : A-Z a-z 0-9 . _ -)");
        close(fd);
        return;
    }

    size_t size = (size_t)st.st_size;
    if (sendf(sockfd, "/send %s %s %zu", nick, name, size) == -1) stop("send");
    char buf[8192];
    size_t sent = 0;
    while (sent < size) {
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n == -1 && errno == EINTR) continue;
        if (n <= 0) {
            /* fichier raccourci pendant la lecture : on complète pour ne pas
               désynchroniser le protocole */
            memset(buf, 0, sizeof(buf));
            n = (ssize_t)(size - sent < sizeof(buf) ? size - sent : sizeof(buf));
        }
        if ((size_t)n > size - sent) n = (ssize_t)(size - sent);
        if (send_all(sockfd, buf, (size_t)n) == -1) stop("send");
        sent += (size_t)n;
    }
    close(fd);
}

/* « /file expéditeur nom taille » : début d'un fichier reçu */
static void start_file(char *args)
{
    char *from = next_word(&args), *name = next_word(&args), *size_s = next_word(&args);
    size_t size;
    if (from == NULL || name == NULL || !parse_size(size_s, &size)) {
        ui_info("En-tête de fichier invalide reçu du serveur");
        return;
    }
    snprintf(file_from, sizeof(file_from), "%s", from);
    file_left = size;
    file_fd = ui_create_file(name, file_path, sizeof(file_path));
    if (file_fd == -1)
        ui_info("Fichier %s de %s refusé (nom invalide ou impossible à créer) : contenu ignoré", name, from);
    if (file_left == 0 && file_fd != -1) {
        close(file_fd);
        file_fd = -1;
        ui_info("Fichier vide %s reçu de %s", file_path, file_from);
    }
}

static void receive_file_bytes(void)
{
    char buf[LINEBUF_SIZE];
    size_t n = linebuf_take(&sock_in, buf, file_left < sizeof(buf) ? file_left : sizeof(buf));
    if (file_fd != -1) {
        size_t off = 0;
        while (off < n) {
            ssize_t w = write(file_fd, buf + off, n - off);
            if (w == -1) {
                if (errno == EINTR) continue;
                ui_info("Écriture de %s impossible : %s", file_path, strerror(errno));
                close(file_fd);
                file_fd = -1;
                break;
            }
            off += (size_t)w;
        }
    }
    file_left -= n;
    if (file_left == 0 && file_fd != -1) {
        close(file_fd);
        file_fd = -1;
        ui_info("Fichier reçu de %s, enregistré dans %s", file_from, file_path);
    }
}

/* traite tout ce qui est arrivé du serveur ; la dernière ligne affichée est copiée dans last */
static void process_socket(char *last, size_t lastsize)
{
    char line[MSG_MAX + 256];
    for (;;) {
        if (file_left > 0) {
            if (sock_in.len == 0) return;
            receive_file_bytes();
        } else if (linebuf_getline(&sock_in, line, sizeof(line))) {
            if (strncmp(line, "/file ", 6) == 0) {
                start_file(line + 6);
            } else {
                ui_print(line);
                if (last != NULL) snprintf(last, lastsize, "%s", line);
            }
        } else {
            return;
        }
    }
}

static void read_socket(char *last, size_t lastsize)
{
    ssize_t n = linebuf_fill(sockfd, &sock_in);
    if (n == 0 || n == -1) {
        ui_info("Connexion fermée par le serveur");
        exit(n == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
    }
    process_socket(last, lastsize);
}

/* lit une ligne au clavier (bloquant) ; 0 en fin d'entrée (Ctrl+D) */
static int read_keyboard_line(char *line, size_t size)
{
    while (!linebuf_getline(&kbd_in, line, size)) {
        ssize_t n = read(STDIN_FILENO, kbd_in.data + kbd_in.len, sizeof(kbd_in.data) - kbd_in.len);
        if (n == -1 && errno == EINTR) continue;
        if (n <= 0) return 0;
        kbd_in.len += (size_t)n;
        if (kbd_in.len == sizeof(kbd_in.data)) kbd_in.len = 0;  /* ligne trop longue */
    }
    return 1;
}

/* ex. 18.2 : demande un pseudo jusqu'à ce que le serveur l'accepte */
static void choose_nick(void)
{
    char line[MSG_MAX + 1], last[MSG_MAX + 256];
    for (;;) {
        printf("Pseudo (ou « pseudo mot-de-passe » s'il est enregistré) : ");
        fflush(stdout);
        if (!read_keyboard_line(line, sizeof(line))) exit(EXIT_SUCCESS);
        if (line[0] == '\0') continue;
        if (sendf(sockfd, "/nickname %s", line) == -1) stop("send");

        /* on attend la réponse du serveur à /nickname */
        last[0] = '\0';
        while (strncmp(last, "*** Vous êtes", 13) != 0 && strncmp(last, "*** Erreur", 10) != 0)
            read_socket(last, sizeof(last));
        if (strncmp(last, "*** Vous êtes", 13) == 0) return;
    }
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "Usage : %s ip port [serveur-ntp|none]\n", argv[0]);
        return EXIT_FAILURE;
    }
    uint16_t port = parse_port(argv[2]);
    const char *ntp_server = (argc > 3) ? argv[3] : "fr.pool.ntp.org";
    signal(SIGPIPE, SIG_IGN);

    /* ex. 18.3 : heure via NTP */
    if (strcmp(ntp_server, "none") == 0)
        ui_info("NTP désactivé : heure de l'horloge locale");
    else if (ntp_init(ntp_server) == 0)
        ui_info("Heure synchronisée avec %s", ntp_server);
    else
        ui_info("NTP indisponible : heure de l'horloge locale");

    /* ex. 18.1 : IP et port en arguments */
    sockfd = tcp_connect(argv[1], port);
    ui_info("Connecté à %s:%u", argv[1], port);

    choose_nick();

    struct pollfd fds[2] = {
        { .fd = STDIN_FILENO, .events = POLLIN },
        { .fd = sockfd,       .events = POLLIN },
    };
    char line[MSG_MAX + 1];
    int stdin_open = 1;
    for (;;) {
        /* il peut rester des lignes tapées pendant le choix du pseudo */
        while (stdin_open && linebuf_getline(&kbd_in, line, sizeof(line))) {
            if (line[0] == '\0') continue;
            if (strncmp(line, "/send ", 6) == 0) {
                send_file(line + 6);
            } else if (sendf(sockfd, "%s", line) == -1) {
                stop("send");
            }
        }

        fds[0].fd = stdin_open ? STDIN_FILENO : -1;   /* fd négatif : ignoré par poll */
        if (poll(fds, 2, -1) == -1) {
            if (errno == EINTR) continue;
            stop("poll");
        }
        if (fds[1].revents & (POLLIN | POLLHUP | POLLERR))
            read_socket(NULL, 0);
        if (fds[0].revents & (POLLIN | POLLHUP | POLLERR)) {
            ssize_t n = read(STDIN_FILENO, kbd_in.data + kbd_in.len, sizeof(kbd_in.data) - kbd_in.len);
            if (n <= 0) {
                /* Ctrl+D : on quitte proprement, le serveur fermera la connexion */
                stdin_open = 0;
                if (sendf(sockfd, "/exit") == -1) stop("send");
            } else {
                kbd_in.len += (size_t)n;
                if (kbd_in.len == sizeof(kbd_in.data)) kbd_in.len = 0;
            }
        }
    }
}
