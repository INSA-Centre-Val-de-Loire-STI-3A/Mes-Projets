/* Découpage des commandes et validation des entrées. */

#include "protocol.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

char *next_word(char **s)
{
    char *p = *s;
    while (*p == ' ') p++;
    if (*p == '\0') {
        *s = p;
        return NULL;
    }
    char *start = p;
    while (*p != '\0' && *p != ' ') p++;
    if (*p == ' ') *p++ = '\0';
    *s = p;
    return start;
}

char *rest_of_line(char **s)
{
    char *p = *s;
    while (*p == ' ') p++;
    *s = p;
    return p;
}

static int valid_name(const char *s, size_t max)
{
    size_t len = strlen(s);
    if (len == 0 || len > max) return 0;
    for (size_t i = 0; i < len; i++)
        if (!isalnum((unsigned char)s[i]) && s[i] != '_' && s[i] != '-')
            return 0;
    return 1;
}

int valid_nick(const char *s)    { return valid_name(s, NICK_MAX); }
int valid_channel(const char *s) { return valid_name(s, CHANNEL_MAX); }

int valid_filename(const char *s)
{
    size_t len = strlen(s);
    if (len == 0 || len > FILENAME_MAX_LEN || s[0] == '.') return 0;
    for (size_t i = 0; i < len; i++)
        if (!isalnum((unsigned char)s[i]) && s[i] != '_' && s[i] != '-' && s[i] != '.')
            return 0;
    return 1;
}

int parse_size(const char *s, size_t *size)
{
    if (s == NULL || *s == '\0') return 0;
    for (const char *p = s; *p; p++)
        if (!isdigit((unsigned char)*p)) return 0;
    errno = 0;
    unsigned long long v = strtoull(s, NULL, 10);
    if (errno != 0 || v > FILE_MAX) return 0;
    *size = (size_t)v;
    return 1;
}

void sanitize(char *s)
{
    for (; *s; s++)
        if ((unsigned char)*s < 0x20 || *s == 0x7f)
            *s = '?';
}

const char *ansi_color(const char *name)
{
    static const struct { const char *name, *code; } colors[] = {
        { "red",     "\033[31m" }, { "green", "\033[32m" }, { "yellow", "\033[33m" },
        { "blue",    "\033[34m" }, { "magenta", "\033[35m" }, { "cyan", "\033[36m" },
    };
    for (size_t i = 0; i < sizeof(colors) / sizeof(colors[0]); i++)
        if (strcmp(name, colors[i].name) == 0)
            return colors[i].code;
    return NULL;
}
