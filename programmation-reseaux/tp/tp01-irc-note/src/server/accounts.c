/* Pseudos enregistrés, sauvegardés dans un fichier « pseudo hash ».
   Le mot de passe n'est jamais stocké : seulement son hachage crypt()
   (yescrypt avec sel aléatoire, comme /etc/shadow). */

#include "accounts.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <crypt.h>

#include "protocol.h"

#define MAX_ACCOUNTS 256
#define HASH_MAX     160

struct account {
    char nick[NICK_MAX + 1];
    char hash[HASH_MAX];
};

static struct account accounts[MAX_ACCOUNTS];
static int  naccounts = 0;
static char file_path[512];

static int find(const char *nick)
{
    for (int i = 0; i < naccounts; i++)
        if (strcmp(accounts[i].nick, nick) == 0)
            return i;
    return -1;
}

/* réécrit tout le fichier : écriture dans un fichier temporaire puis rename(),
   pour ne jamais laisser un fichier à moitié écrit */
static int save(void)
{
    char tmp[sizeof(file_path) + 8];
    snprintf(tmp, sizeof(tmp), "%s.tmp", file_path);
    int fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd == -1) { perror(tmp); return -1; }
    FILE *f = fdopen(fd, "w");
    if (f == NULL) { perror("fdopen"); close(fd); return -1; }
    for (int i = 0; i < naccounts; i++)
        fprintf(f, "%s %s\n", accounts[i].nick, accounts[i].hash);
    if (fclose(f) != 0) { perror(tmp); return -1; }
    if (rename(tmp, file_path) == -1) { perror("rename"); return -1; }
    return 0;
}

void accounts_load(const char *path)
{
    snprintf(file_path, sizeof(file_path), "%s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        if (errno != ENOENT) perror(path);
        return;                        /* premier lancement : aucun compte */
    }
    char line[NICK_MAX + HASH_MAX + 8];
    while (naccounts < MAX_ACCOUNTS && fgets(line, sizeof(line), f) != NULL) {
        char *p = line;
        line[strcspn(line, "\n")] = '\0';
        char *nick = next_word(&p), *hash = next_word(&p);
        if (nick == NULL || hash == NULL || !valid_nick(nick) || strlen(hash) >= HASH_MAX)
            continue;
        snprintf(accounts[naccounts].nick, sizeof(accounts[0].nick), "%s", nick);
        snprintf(accounts[naccounts].hash, sizeof(accounts[0].hash), "%s", hash);
        naccounts++;
    }
    fclose(f);
    printf("%d pseudo(s) enregistré(s) chargé(s) depuis %s\n", naccounts, path);
}

int accounts_exists(const char *nick)
{
    return find(nick) >= 0;
}

/* comparaison en temps constant : le temps de réponse ne révèle pas
   combien de caractères du hachage sont corrects */
static int equal_ct(const char *a, const char *b)
{
    size_t la = strlen(a), lb = strlen(b);
    unsigned char diff = (unsigned char)(la != lb);
    for (size_t i = 0; i < la && i < lb; i++)
        diff |= (unsigned char)(a[i] ^ b[i]);
    return diff == 0;
}

int accounts_check(const char *nick, const char *password)
{
    int i = find(nick);
    if (i < 0) return 0;
    static struct crypt_data data;     /* ~32 Ko : pas sur la pile */
    memset(&data, 0, sizeof(data));
    const char *h = crypt_r(password, accounts[i].hash, &data);
    return h != NULL && h[0] != '*' && equal_ct(h, accounts[i].hash);
}

int accounts_register(const char *nick, const char *password)
{
    if (naccounts >= MAX_ACCOUNTS || find(nick) >= 0) return -1;

    /* préfixe par défaut de libxcrypt (yescrypt) et sel tiré de /dev/urandom */
    char *salt = crypt_gensalt_ra(NULL, 0, NULL, 0);
    if (salt == NULL) { perror("crypt_gensalt"); return -1; }
    static struct crypt_data data;
    memset(&data, 0, sizeof(data));
    const char *h = crypt_r(password, salt, &data);
    free(salt);
    if (h == NULL || h[0] == '*' || strlen(h) >= HASH_MAX) return -1;

    snprintf(accounts[naccounts].nick, sizeof(accounts[0].nick), "%s", nick);
    snprintf(accounts[naccounts].hash, sizeof(accounts[0].hash), "%s", h);
    naccounts++;
    if (save() == -1) { naccounts--; return -1; }
    return 0;
}

int accounts_unregister(const char *nick)
{
    int i = find(nick);
    if (i < 0) return -1;
    struct account removed = accounts[i];
    accounts[i] = accounts[--naccounts];
    if (save() == -1) {                /* on remet le compte en cas d'échec */
        accounts[naccounts++] = accounts[i];
        accounts[i] = removed;
        return -1;
    }
    return 0;
}
