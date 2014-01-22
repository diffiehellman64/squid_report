#ifndef __USERS_REQUEST_H_
#define __USERS_REQUEST_H_

//
#define	USERNAME_MAXLEN 128
#define ORGNAME_MAXLEN 128
#define SITE_MAXLEN 128

typedef struct hash_table user_table_t;

user_table_t *user_table_new();
void user_table_del(user_table_t **table);
void user_table_add_entry(user_table_t *table, char *uname, char *site);
void user_table_list(user_table_t *table);
void user_table_write_csv(user_table_t *table, char **sites, char *csvfile);

#endif
