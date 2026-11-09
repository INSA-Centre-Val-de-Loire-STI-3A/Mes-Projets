/* Pseudos enregistrés : /register, /unregister, /nickname pseudo mot-de-passe.
   Les mots de passe sont hachés avec crypt() (yescrypt, sel aléatoire)
   et sauvegardés dans un fichier texte « pseudo hash » (droits 0600). */
#ifndef ACCOUNTS_H
#define ACCOUNTS_H

void accounts_load(const char *path);                        /* crée le fichier s'il manque */
int  accounts_exists(const char *nick);
int  accounts_check(const char *nick, const char *password); /* 1 si le mot de passe est bon */
int  accounts_register(const char *nick, const char *password);   /* 0 ou -1 */
int  accounts_unregister(const char *nick);                       /* 0 ou -1 */

#endif
