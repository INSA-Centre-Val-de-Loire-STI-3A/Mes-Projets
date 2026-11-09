/* Affichage du client. */

#include "ui.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include "ntp.h"
#include "protocol.h"

void ui_print(const char *line)
{
    time_t t = ntp_now();
    struct tm tm;
    char hour[16];
    strftime(hour, sizeof(hour), "%H:%M:%S", localtime_r(&t, &tm));
    /* les alertes contiennent \a (bip) et des couleurs : le terminal les interprète.
       Remise à zéro de la couleur seulement si la ligne en contient une. */
    printf("[%s] %s%s\n", hour, line, strchr(line, '\033') ? ANSI_RESET : "");
    fflush(stdout);
}

void ui_info(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    printf("--- ");
    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
    fflush(stdout);
}

int ui_create_file(const char *name, char *path, size_t size)
{
    /* le nom vient du réseau : on n'accepte qu'un nom simple, jamais un chemin
       (« ../../.bashrc » écrirait hors du répertoire courant) */
    if (!valid_filename(name))
        return -1;
    for (int i = 0; i < 100; i++) {
        if (i == 0) snprintf(path, size, "%s", name);
        else        snprintf(path, size, "%s.%d", name, i);
        /* O_EXCL : ne jamais écraser un fichier existant (ni suivre un lien) */
        int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
        if (fd != -1 || errno != EEXIST)
            return fd;
    }
    return -1;
}
